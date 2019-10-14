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

bool
RUdpCommandPod::Timeout(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time)
{
    auto cur_cmd = sent_reliable_commands_.begin();

    while (cur_cmd != sent_reliable_commands_.end()) {
        auto out_cmd = cur_cmd;

        ++cur_cmd;

        // 処理をスキップ
        if (UDP_TIME_DIFFERENCE(service_time, (*out_cmd)->sent_time()) < (*out_cmd)->round_trip_timeout())
            continue;

        if (earliest_timeout_ == 0 || UDP_TIME_LESS((*out_cmd)->sent_time(), earliest_timeout_))
            earliest_timeout_ = (*out_cmd)->sent_time();

        // タイムアウトしたらピアを切断する
        auto exceeds_timeout_maximum = UDP_TIME_DIFFERENCE(service_time, earliest_timeout_) >= timeout_maximum_;
        auto exceeds_round_trip_timeout_limit =
            (*out_cmd)->round_trip_timeout() >= (*out_cmd)->round_trip_timeout_limit();
        auto exceeds_timeout_minimum = UDP_TIME_DIFFERENCE(service_time, earliest_timeout_) >= timeout_minimum_;
        if (earliest_timeout_ != 0 &&
            (exceeds_timeout_maximum || (exceeds_round_trip_timeout_limit && exceeds_timeout_minimum)))
        {
            // TODO: call NotifyDisconnect()
            // ...

            core::Singleton<core::Logger>::Instance().Debug("peer is going to be disconnected (timeout)");

            return true;
        }

        if ((*out_cmd)->HasPayload()) {
            reliable_data_in_transit_ -= (*out_cmd)->fragment_length();
        }

        net->IncreaseSegmentsLost(1);
        (*out_cmd)->round_trip_timeout((*out_cmd)->round_trip_timeout() * 2);

        outgoing_reliable_commands_.insert(outgoing_reliable_commands_.begin(), *out_cmd);

        core::Singleton<core::Logger>::Instance().Debug("command is going to send next time (timeout): {0}",
                                                        COMMANDS_AS_STRING.at((*out_cmd)->CommandNumber()));

        if (cur_cmd == sent_reliable_commands_.begin() && !sent_reliable_commands_.empty())
            next_timeout_ = (*cur_cmd)->sent_time() + (*cur_cmd)->round_trip_timeout();
    }

    return false;
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
    core::Singleton<core::Logger>::Instance().Debug("outgoing reliable command count: {0}",
                                                    outgoing_reliable_commands_.size());

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

        chamber->IncrementSegmentSize(
            COMMAND_SIZES[(*outgoing_command)->CommandNumber()]
        );

        auto flags = chamber->header_flags();
        chamber->header_flags(flags | static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME));

        // MEMO: bufferには「コマンド、データ、コマンド、データ・・・」という順番でパケットが挿入される
        //       これは受信側でパケットを正しく識別するための基本

        if ((*outgoing_command)->HasPayload())
        {
            buffer = chamber->EmptyDataBuffer();
            (*buffer)->Add((*outgoing_command)->segment()->Data(), (*outgoing_command)->fragment_offset());

            chamber->IncrementSegmentSize((*outgoing_command)->fragment_length());

            IncrementReliableDataInTransit((*outgoing_command)->fragment_length());
        }

        sent_reliable_commands_.push_back(*outgoing_command);

        outgoing_reliable_commands_.erase(outgoing_command);

        core::Singleton<core::Logger>::Instance().Debug("outgoing reliable command was removed (on send): {0} (ch: {1}, sn: {2})",
                                                        COMMANDS_AS_STRING.at((*outgoing_command)->CommandNumber()),
                                                        (*outgoing_command)->command()->header.channel_id,
                                                        ntohs((*outgoing_command)->command()->header.reliable_sequence_number));

        core::Singleton<core::Logger>::Instance().Debug("outgoing reliable command count: {0} ({1})",
                                                        outgoing_reliable_commands_.size(),
                                                        ntohs((*outgoing_command)->command()->header.reliable_sequence_number));
    }

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
        auto command_size = COMMAND_SIZES[(*outgoing_command)->CommandNumber()];

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

        chamber->IncrementSegmentSize(command_size);

        outgoing_unreliable_commands_.erase(outgoing_command);

        if ((*outgoing_command)->HasPayload())
        {
            buffer = chamber->EmptyDataBuffer();
            (*buffer)->Add((*outgoing_command)->segment()->Data(), (*outgoing_command)->fragment_offset());

            chamber->IncrementSegmentSize((*outgoing_command)->fragment_length());

            sent_unreliable_commands_.push_back(*outgoing_command);
        }
    }

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

    // TODO: next_timeout_ の更新はメソッド化できる
    next_timeout_ = outgoing_command->NextTimeout();

    if (no_sent_reliable_command_matched)
    {
        outgoing_reliable_commands_.erase(it);
        core::Singleton<core::Logger>::Instance().Debug("outgoing reliable command was removed (on receive): {0} ({1})",
                                                        COMMANDS_AS_STRING.at(outgoing_command->CommandNumber()),
                                                        reliable_sequence_number);
    }
    else
    {
        sent_reliable_commands_.erase(it);
        core::Singleton<core::Logger>::Instance().Debug("sent reliable command was removed (on receive): {0} ({1})",
                                                        COMMANDS_AS_STRING.at(outgoing_command->CommandNumber()),
                                                        reliable_sequence_number);
    }

    return static_cast<RUdpProtocolCommand>(command_number);
}

void
RUdpCommandPod::RemoveSentUnreliableCommands()
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
RUdpCommandPod::SetupOutgoingCommand(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command,
                                     const std::shared_ptr<RUdpChannel> &channel)
{
    outgoing_data_total_ +=
        COMMAND_SIZES.at(outgoing_command->CommandNumber()) + outgoing_command->fragment_length();

    if (channel == nullptr)
    {
        IncrementOutgoingReliableSequenceNumber();

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
    {
        outgoing_reliable_commands_.push_back(outgoing_command);
        core::Singleton<core::Logger>::Instance().Debug("outgoing reliable command was added: {0} ({1})",
                                                        COMMANDS_AS_STRING.at(outgoing_command->CommandNumber()),
                                                        ntohs(outgoing_command->command()->header.reliable_sequence_number));
    }
    else
    {
        outgoing_unreliable_commands_.push_back(outgoing_command);
        core::Singleton<core::Logger>::Instance().Debug("outgoing unreliable command was added: {0} (-)",
                                                        COMMANDS_AS_STRING.at(outgoing_command->CommandNumber()));
    }
}
