#include "lib/rudp/RUdpEnum.h"
#include "RUdpCommandPod.h"
#include "RUdpCommandSize.h"

namespace
{
bool
window_wraps(const std::shared_ptr<RUdpChannel> &channel,
             int reliable_window,
             const std::shared_ptr<RUdpOutgoingCommand> &outgoing_command)
{
    auto has_not_sent_once = outgoing_command->send_attempts() == 0;

    auto first_command_in_window = !(outgoing_command->reliable_sequence_number() % PEER_RELIABLE_WINDOW_SIZE);

    auto all_available_windows_are_in_use = channel->ReliableWindow(
        (reliable_window + PEER_RELIABLE_WINDOWS - 1) % PEER_RELIABLE_WINDOWS
    ) >= PEER_RELIABLE_WINDOW_SIZE;

    auto existing_commands_are_in_flight = channel->used_reliable_windows() & (
        (((1 << PEER_FREE_RELIABLE_WINDOWS) - 1) << reliable_window) |
            (((1 << PEER_FREE_RELIABLE_WINDOWS) - 1) >> (PEER_RELIABLE_WINDOWS - reliable_window))
    );

    return has_not_sent_once &&
        first_command_in_window &&
        (all_available_windows_are_in_use || existing_commands_are_in_flight);
}

bool
window_exceeds(uint32_t reliable_data_in_transit,
               uint32_t mtu,
               uint32_t window_size,
               const std::shared_ptr<RUdpOutgoingCommand> &outgoing_command)
{
    return (reliable_data_in_transit + outgoing_command->fragment_length()) > std::max(window_size, mtu);
}
}

RUdpCommandPod::RUdpCommandPod()
    : incoming_data_total_(),
      outgoing_data_total_(),
      outgoing_reliable_sequence_number_(),
      incoming_unsequenced_group_(),
      outgoing_unsequenced_group_(),
      round_trip_time_(),
      round_trip_time_variance_(),
      timeout_limit_(),
      next_timeout_(),
      earliest_timeout_(),
      timeout_minimum_(),
      timeout_maximum_(),
      reliable_data_in_transit_(),
      outgoing_reliable_commands_(),
      outgoing_unreliable_commands_(),
      sent_reliable_commands_(),
      sent_unreliable_commands_()
{}

void
RUdpCommandPod::SetupOutgoingCommand(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command,
                                     const std::shared_ptr<RUdpChannel> &channel)
{
    outgoing_data_total_ +=
        command_sizes.at(outgoing_command->CommandNumber()) + outgoing_command->fragment_length();

    if (channel == nullptr)
    {
        ++outgoing_reliable_sequence_number_;

        outgoing_command->reliable_sequence_number(outgoing_reliable_sequence_number_);
        outgoing_command->unreliable_sequence_number(0);
    }
    else if (outgoing_command->IsAcknowledge())
    {
        channel->IncrementOutgoingReliableSequenceNumber();
        channel->outgoing_unreliable_sequence_number(0);

        outgoing_command->reliable_sequence_number(channel->outgoing_reliable_sequence_number());
        outgoing_command->unreliable_sequence_number(0);
    }
    else if (outgoing_command->IsUnsequenced())
    {
        ++outgoing_unsequenced_group_;

        outgoing_command->reliable_sequence_number(0);
        outgoing_command->unreliable_sequence_number(0);
    }
    else
    {
        if (outgoing_command->fragment_offset() == 0)
            channel->IncrementOutgoingUnreliableSequenceNumber();

        outgoing_command->reliable_sequence_number(channel->outgoing_reliable_sequence_number());
        outgoing_command->unreliable_sequence_number(channel->outgoing_unreliable_sequence_number());
    }

    outgoing_command->send_attempts(0);
    outgoing_command->sent_time(0);
    outgoing_command->round_trip_timeout(0);
    outgoing_command->round_trip_timeout_limit(0);
    outgoing_command->header_reliable_sequence_number(htons(outgoing_command->reliable_sequence_number()));

    auto cmd = outgoing_command->CommandNumber();

    if (static_cast<RUdpProtocolCommand>(cmd) == RUdpProtocolCommand::SEND_UNRELIABLE) {
        outgoing_command->send_unreliable_unreliable_sequence_number(
            htons(outgoing_command->unreliable_sequence_number())
        );
    }
    else if (static_cast<RUdpProtocolCommand>(cmd) == RUdpProtocolCommand::SEND_UNSEQUENCED) {
        outgoing_command->send_unsequenced_unsequenced_group(htons(outgoing_unsequenced_group_));
    }

    if (outgoing_command->IsAcknowledge())
        outgoing_reliable_commands_.push_back(outgoing_command);
    else
        outgoing_unreliable_commands_.push_back(outgoing_command);
}

/* enet_protocol_send_reliable_outgoing_commands()
 *
 * */
bool
RUdpCommandPod::LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber,
                                                std::unique_ptr<RUdpPeerNet> &net,
                                                const std::vector<std::shared_ptr<RUdpChannel>> &channels,
                                                uint32_t service_time)
{
    auto window_exceeded = 0;
    auto window_wrap = false;
    auto can_ping = true;
    auto current_command = outgoing_reliable_commands_.begin();

    while (current_command != outgoing_reliable_commands_.end()) {
        auto command = chamber->EmptyCommandBuffer();
        auto buffer = chamber->EmptyDataBuffer();

        auto outgoing_command = current_command;

        auto channel = (*outgoing_command)->header_channel_id() < channels.size() ?
                       channels.at((*outgoing_command)->header_channel_id()) :
                       nullptr;

        auto reliable_window = (*outgoing_command)->reliable_sequence_number() / PEER_RELIABLE_WINDOW_SIZE;

        //  check reliable window is available
        // --------------------------------------------------

        if (channel != nullptr) {
            if (!window_wrap && window_wraps(channel, reliable_window, (*outgoing_command)))
                window_wrap = true;

            if (window_wrap) {
                ++current_command;

                continue;
            }
        }

        //  check segment exceeds window size
        // --------------------------------------------------

        if ((*outgoing_command)->HasPayload()) {
            if (!window_exceeded) {
                auto ws = (net->segment_throttle() * net->window_size()) / PEER_SEGMENT_THROTTLE_SCALE;

                if (window_exceeds(reliable_data_in_transit_, net->mtu(), ws, (*outgoing_command)))
                    window_exceeded = true;
            }

            if (window_exceeded) {
                ++current_command;

                continue;
            }
        }

        //
        // --------------------------------------------------

        can_ping = false;

        if (chamber->SendingContinues(command, buffer, net->mtu(), (*outgoing_command))) {
            chamber->continue_sending(true);

            break;
        }

        ++current_command;

        if (channel != nullptr && (*outgoing_command)->send_attempts() < 1) {
            channel->MarkReliableWindowAsUsed(reliable_window);
            channel->IncrementReliableWindow(reliable_window);
        }

        (*outgoing_command)->IncrementSendAttempts();

        if ((*outgoing_command)->round_trip_timeout() == 0) {
            (*outgoing_command)->round_trip_timeout(round_trip_time_ + 4 * round_trip_time_variance_);
            (*outgoing_command)->round_trip_timeout_limit(timeout_limit_ * (*outgoing_command)->round_trip_timeout());
        }

        if (!sent_reliable_commands_.empty())
            next_timeout_ = service_time + (*outgoing_command)->round_trip_timeout();

        (*outgoing_command)->sent_time(service_time);

        (*buffer)->Add((*outgoing_command)->command());
        //buffer->Add(command);
        //buffer->data_length = command_sizes[(*outgoing_command)->command->header.command & PROTOCOL_COMMAND_MASK];

        chamber->IncrementSegmentSize(
            command_sizes[(*outgoing_command)->CommandNumber()]
        );

        auto flags = chamber->header_flags();
        chamber->header_flags(flags | static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME));

        // MEMO: bufferには「コマンド、データ、コマンド、データ・・・」という順番でパケットが挿入される
        //       これは受信側でパケットを正しく識別するための基本

        //*command = *(*outgoing_command)->command;

        if ((*outgoing_command)->HasPayload()) {
            //++buffer;
            buffer = chamber->EmptyDataBuffer();

            (*buffer)->Add((*outgoing_command)->segment()->Data(), (*outgoing_command)->fragment_offset());
            //buffer->data_length = (*outgoing_command)->fragment_length;

            chamber->IncrementSegmentSize((*outgoing_command)->fragment_length());

            increse_reliable_data_in_transit((*outgoing_command)->fragment_length());
        }

        sent_reliable_commands_.push_back(*outgoing_command);

        outgoing_reliable_commands_.erase(outgoing_command);

        // MEMO: push_sent_reliable_command() でインクリメント
        //++segments_sent_;

        //++command;
        //++buffer;
    }

    //chamber->update_command_count(command);
    //chamber->update_buffer_count(buffer);

    return can_ping;
}

bool
RUdpCommandPod::LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber,
                                                  std::unique_ptr<RUdpPeerNet> &net)
{
    auto can_disconnect = false;
    auto current_command = outgoing_unreliable_commands_.begin();

    while (current_command != outgoing_unreliable_commands_.end()) {
        auto command = chamber->EmptyCommandBuffer();
        auto buffer = chamber->EmptyDataBuffer();

        auto outgoing_command = current_command;
        auto command_size = command_sizes[(*outgoing_command)->CommandNumber()];

        if (chamber->SendingContinues(command, buffer, net->mtu(), (*outgoing_command))) {
            chamber->continue_sending(true);

            break;
        }

        ++current_command;

        if ((*outgoing_command)->HasPayload() && (*outgoing_command)->fragment_offset() == 0) {
            net->UpdateSegmentThrottleCounter();

            if (net->ExceedsSegmentThrottleCounter()) {
                uint16_t reliable_sequence_number = (*outgoing_command)->reliable_sequence_number();
                uint16_t unreliable_sequence_number = (*outgoing_command)->unreliable_sequence_number();

                for (;;) {
                    if (current_command == outgoing_unreliable_commands_.end())
                        break;

                    outgoing_command = current_command;

                    if ((*outgoing_command)->reliable_sequence_number() != reliable_sequence_number ||
                        (*outgoing_command)->unreliable_sequence_number() != unreliable_sequence_number) {
                        break;
                    }

                    ++current_command;
                }

                continue;
            }
        }

        *command = (*outgoing_command)->command();

        (*buffer)->Add(*command);
        //buffer->data_length = command_size;

        chamber->IncrementSegmentSize(command_size);

        outgoing_unreliable_commands_.erase(outgoing_command);

        if ((*outgoing_command)->HasPayload()) {
            //++buffer;
            buffer = chamber->EmptyDataBuffer();

            (*buffer)->Add((*outgoing_command)->segment()->Data(), (*outgoing_command)->fragment_offset());
            //buffer->data_length = (*outgoing_command)->fragment_length;

            chamber->IncrementSegmentSize((*outgoing_command)->fragment_length());

            sent_unreliable_commands_.push_back(*outgoing_command);
        }

        //++command;
        //++buffer;
    }

    //chamber->update_command_count(command);
    //chamber->update_buffer_count(buffer);

    // TODO: stateやthrottle関連のプロパティは新しいクラスにまとめたい（このクラスはRUdpPeerが所有する）
    if (net->StateIs(RUdpPeerState::DISCONNECT_LATER) &&
        outgoing_reliable_commands_.empty() &&
        outgoing_unreliable_commands_.empty() &&
        !sent_reliable_commands_.empty()) {
        can_disconnect = true;
    }

    // ↑のロジックの結果次第でtrue/falseを返す
    // PurgePeer() はRUdpPeerPodから呼ぶ
    return can_disconnect;
}

void
RUdpCommandPod::Reset()
{
    incoming_data_total_ = 0;
    outgoing_data_total_ = 0;
    next_timeout_ = 0;
    timeout_limit_ = PEER_TIMEOUT_LIMIT;
    round_trip_time_ = PEER_DEFAULT_ROUND_TRIP_TIME;
    round_trip_time_variance_ = 0;
    outgoing_reliable_sequence_number_ = 0;
    incoming_unsequenced_group_ = 0;
    outgoing_unsequenced_group_ = 0;
    earliest_timeout_ = 0;
    timeout_minimum_ = PEER_TIMEOUT_MINIMUM;
    timeout_maximum_ = PEER_TIMEOUT_MAXIMUM;
    reliable_data_in_transit_ = 0;
}

void
RUdpCommandPod::increse_reliable_data_in_transit(uint32_t val)
{
    reliable_data_in_transit_ += val;
}

bool
RUdpCommandPod::sent_reliable_command_exists()
{
    return !sent_reliable_commands_.empty();
}

void
RUdpCommandPod::clear_sent_reliable_command()
{
    sent_reliable_commands_.clear();
}

void
RUdpCommandPod::clear_sent_unreliable_command()
{
    sent_unreliable_commands_.clear();
}

int
RUdpCommandPod::check_timeouts(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time)
{
    auto current_command = sent_reliable_commands_.begin();

    while (current_command != sent_reliable_commands_.end()) {
        auto outgoing_command = current_command;

        ++current_command;

        // 処理をスキップ
        if (UDP_TIME_DIFFERENCE(service_time, (*outgoing_command)->sent_time()) < (*outgoing_command)->round_trip_timeout())
            continue;

        if (earliest_timeout_ == 0 || UDP_TIME_LESS((*outgoing_command)->sent_time(), earliest_timeout_))
            earliest_timeout_ = (*outgoing_command)->sent_time();

        // タイムアウトしたらピアを切断する
        if (earliest_timeout_ != 0 &&
            (UDP_TIME_DIFFERENCE(service_time, earliest_timeout_) >= timeout_maximum_ ||
                ((*outgoing_command)->round_trip_timeout() >= (*outgoing_command)->round_trip_timeout_limit() &&
                    UDP_TIME_DIFFERENCE(service_time, earliest_timeout_) >= timeout_minimum_))) {
            return 1;
        }

        if ((*outgoing_command)->HasPayload()) {
            reliable_data_in_transit_ -= (*outgoing_command)->fragment_length();
        }

        net->IncreaseSegmentsLost(1);

        (*outgoing_command)->round_trip_timeout((*outgoing_command)->round_trip_timeout() * 2);

        outgoing_reliable_commands_.push_back(*outgoing_command);

        // TODO: ENetの条件式とは違うため、要検証（おそらく意味は同じであるはず）
        if (!sent_reliable_commands_.empty() && sent_reliable_commands_.size() == 1) {
            next_timeout_ = (*current_command)->sent_time() + (*current_command)->round_trip_timeout();
        }
    }

    return 0;
}

RUdpProtocolCommand
RUdpCommandPod::RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id,
    std::shared_ptr<RUdpChannel> &channel)
{
    std::shared_ptr<RUdpOutgoingCommand> outgoing_command;
    auto it = sent_reliable_commands_.begin();
    auto it_end = sent_reliable_commands_.end();
    auto no_sent_reliable_command_matched = false;

    for (; it != it_end; ++it)
    {
        if ((*it)->reliable_sequence_number() == reliable_sequence_number &&
            (*it)->header_channel_id() == channel_id)
        {
            outgoing_command = (*it);
            break;
        }

        if (it == sent_reliable_commands_.end())
            no_sent_reliable_command_matched = true;
    }

    auto was_sent = true;

    if (no_sent_reliable_command_matched)
    {
        it = outgoing_reliable_commands_.begin();
        it_end = outgoing_reliable_commands_.end();

        for (; it != it_end; ++it)
        {
            if ((*it)->send_attempts() < 1)
                return RUdpProtocolCommand::NONE;

            if (((*it)->reliable_sequence_number() == reliable_sequence_number) &&
                ((*it)->header_channel_id() == channel_id))
            {
                outgoing_command = (*it);
                break;
            }
        }

        if (it == it_end)
            return RUdpProtocolCommand::NONE;

        was_sent = false;
    }

    if (outgoing_command == nullptr)
        return RUdpProtocolCommand::NONE;

    if (channel)
    {
        auto reliable_window = reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;
        if (channel->ReliableWindow(reliable_window) > 0)
        {
            channel->DecrementReliableWindow(reliable_window);
            if (channel->ReliableWindow(reliable_window) == 0)
                channel->MarkReliableWindowAsUnused(reliable_window);
        }
    }

    auto command_number = outgoing_command->CommandNumber();

    if (outgoing_command->HasPayload())
    {
        if (was_sent)
            reliable_data_in_transit_ -= outgoing_command->fragment_length();
    }

    if (sent_reliable_commands_.empty())
        return static_cast<RUdpProtocolCommand>(command_number);

    outgoing_command = sent_reliable_commands_.front();

    next_timeout_ = outgoing_command->sent_time() + outgoing_command->round_trip_timeout();

    if (no_sent_reliable_command_matched)
        outgoing_reliable_commands_.erase(it);
    else
        sent_reliable_commands_.erase(it);

    return static_cast<RUdpProtocolCommand>(command_number);
}

void
RUdpCommandPod::remove_sent_unreliable_commands()
{
    while (!sent_unreliable_commands_.empty()) {
        auto &outgoing_command = sent_unreliable_commands_.front();

        if (outgoing_command->HasPayload()) {
            if (outgoing_command->segment().use_count() == 1)
                outgoing_command->segment()->AddFlag(static_cast<uint32_t>(RUdpSegmentFlag::SENT));

            outgoing_command->segment()->Destroy();
        }

        sent_unreliable_commands_.pop_front();
    }
}
