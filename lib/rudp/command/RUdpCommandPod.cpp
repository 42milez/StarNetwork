#include "lib/rudp/RUdpSegmentFlag.h"
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
    : _incoming_data_total(),
      _outgoing_data_total(),
      _outgoing_reliable_sequence_number(),
      _incoming_unsequenced_group(),
      _outgoing_unsequenced_group(),
      _round_trip_time(),
      _round_trip_time_variance(),
      _timeout_limit(),
      _next_timeout(),
      _earliest_timeout(),
      _timeout_minimum(),
      _timeout_maximum(),
      _reliable_data_in_transit(),
      _outgoing_reliable_commands(),
      _outgoing_unreliable_commands(),
      _sent_reliable_commands(),
      _sent_unreliable_commands()
{}

void
RUdpCommandPod::setup_outgoing_command(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command,
                                       const std::shared_ptr<RUdpChannel> &channel)
{
    _outgoing_data_total +=
        command_sizes.at(outgoing_command->CommandNumber()) + outgoing_command->fragment_length();

    if (channel == nullptr)
    {
        ++_outgoing_reliable_sequence_number;

        outgoing_command->reliable_sequence_number(_outgoing_reliable_sequence_number);
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
        ++_outgoing_unsequenced_group;

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
        outgoing_command->send_unsequenced_unsequenced_group(htons(_outgoing_unsequenced_group));
    }

    if (outgoing_command->IsAcknowledge())
        _outgoing_reliable_commands.push_back(outgoing_command);
    else
        _outgoing_unreliable_commands.push_back(outgoing_command);
}

void
RUdpCommandPod::push_outgoing_reliable_command(std::shared_ptr<RUdpOutgoingCommand> &command)
{
    _outgoing_reliable_commands.push_front(command);
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
    auto current_command = _outgoing_reliable_commands.begin();

    while (current_command != _outgoing_reliable_commands.end()) {
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

                if (window_exceeds(_reliable_data_in_transit, net->mtu(), ws, (*outgoing_command)))
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
            (*outgoing_command)->round_trip_timeout(_round_trip_time + 4 * _round_trip_time_variance);
            (*outgoing_command)->round_trip_timeout_limit(_timeout_limit * (*outgoing_command)->round_trip_timeout());
        }

        if (!_sent_reliable_commands.empty())
            _next_timeout = service_time + (*outgoing_command)->round_trip_timeout();

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

        _sent_reliable_commands.push_back(*outgoing_command);

        _outgoing_reliable_commands.erase(outgoing_command);

        // MEMO: push_sent_reliable_command() でインクリメント
        //++_segments_sent;

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
    auto current_command = _outgoing_unreliable_commands.begin();

    while (current_command != _outgoing_unreliable_commands.end()) {
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
            net->update_segment_throttle_counter();

            if (net->exceeds_segment_throttle_counter()) {
                uint16_t reliable_sequence_number = (*outgoing_command)->reliable_sequence_number();
                uint16_t unreliable_sequence_number = (*outgoing_command)->unreliable_sequence_number();

                for (;;) {
                    if (current_command == _outgoing_unreliable_commands.end())
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

        _outgoing_unreliable_commands.erase(outgoing_command);

        if ((*outgoing_command)->HasPayload()) {
            //++buffer;
            buffer = chamber->EmptyDataBuffer();

            (*buffer)->Add((*outgoing_command)->segment()->Data(), (*outgoing_command)->fragment_offset());
            //buffer->data_length = (*outgoing_command)->fragment_length;

            chamber->IncrementSegmentSize((*outgoing_command)->fragment_length());

            _sent_unreliable_commands.push_back(*outgoing_command);
        }

        //++command;
        //++buffer;
    }

    //chamber->update_command_count(command);
    //chamber->update_buffer_count(buffer);

    // TODO: stateやthrottle関連のプロパティは新しいクラスにまとめたい（このクラスはRUdpPeerが所有する）
    if (net->StateIs(RUdpPeerState::DISCONNECT_LATER) &&
        _outgoing_reliable_commands.empty() &&
        _outgoing_unreliable_commands.empty() &&
        !_sent_reliable_commands.empty()) {
        can_disconnect = true;
    }

    // ↑のロジックの結果次第でtrue/falseを返す
    // PurgePeer() はRUdpPeerPodから呼ぶ
    return can_disconnect;
}

uint32_t
RUdpCommandPod::outgoing_data_total()
{
    return _outgoing_data_total;
}

void
RUdpCommandPod::outgoing_data_total(uint32_t val)
{
    _outgoing_data_total += val;
}

uint32_t
RUdpCommandPod::incoming_data_total()
{
    return _incoming_data_total;
}

void
RUdpCommandPod::incoming_data_total(uint32_t val)
{
    _incoming_data_total += val;
}

uint32_t
RUdpCommandPod::next_timeout()
{
    return _next_timeout;
}

bool
RUdpCommandPod::outgoing_reliable_command_exists()
{
    return !_outgoing_reliable_commands.empty();
}

bool
RUdpCommandPod::outgoing_unreliable_command_exists()
{
    return !_outgoing_unreliable_commands.empty();
}

void
RUdpCommandPod::clear_outgoing_reliable_command()
{
    _outgoing_reliable_commands.clear();
}

void
RUdpCommandPod::clear_outgoing_unreliable_command()
{
    _outgoing_unreliable_commands.clear();
}

void
RUdpCommandPod::Reset()
{
    _incoming_data_total = 0;
    _outgoing_data_total = 0;
    _next_timeout = 0;
    _timeout_limit = PEER_TIMEOUT_LIMIT;
    _round_trip_time = PEER_DEFAULT_ROUND_TRIP_TIME;
    _round_trip_time_variance = 0;
    _outgoing_reliable_sequence_number = 0;
    _incoming_unsequenced_group = 0;
    _outgoing_unsequenced_group = 0;
    _earliest_timeout = 0;
    _timeout_minimum = PEER_TIMEOUT_MINIMUM;
    _timeout_maximum = PEER_TIMEOUT_MAXIMUM;
    _reliable_data_in_transit = 0;
}

void
RUdpCommandPod::increse_reliable_data_in_transit(uint32_t val)
{
    _reliable_data_in_transit += val;
}

uint32_t
RUdpCommandPod::reliable_data_in_transit()
{
    return _reliable_data_in_transit;
}

void
RUdpCommandPod::reliable_data_in_transit(uint32_t val)
{
    _reliable_data_in_transit = val;
}

void
RUdpCommandPod::next_timeout(uint32_t val)
{
    _next_timeout = val;
}

void
RUdpCommandPod::sent_reliable_command(std::shared_ptr<RUdpOutgoingCommand> &command, std::unique_ptr<RUdpPeerNet> &net)
{
    _sent_reliable_commands.push_back(command);

    net->increase_segments_sent(1);
}

void
RUdpCommandPod::sent_unreliable_command(std::shared_ptr<RUdpOutgoingCommand> &command)
{
    _sent_unreliable_commands.push_back(command);
}

bool
RUdpCommandPod::sent_reliable_command_exists()
{
    return !_sent_reliable_commands.empty();
}

void
RUdpCommandPod::clear_sent_reliable_command()
{
    _sent_reliable_commands.clear();
}

bool
RUdpCommandPod::sent_unreliable_command_exists()
{
    return !_sent_unreliable_commands.empty();
}

void
RUdpCommandPod::clear_sent_unreliable_command()
{
    _sent_unreliable_commands.clear();
}

int
RUdpCommandPod::check_timeouts(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time)
{
    auto current_command = _sent_reliable_commands.begin();

    while (current_command != _sent_reliable_commands.end()) {
        auto outgoing_command = current_command;

        ++current_command;

        // 処理をスキップ
        if (UDP_TIME_DIFFERENCE(service_time, (*outgoing_command)->sent_time()) < (*outgoing_command)->round_trip_timeout())
            continue;

        if (_earliest_timeout == 0 || UDP_TIME_LESS((*outgoing_command)->sent_time(), _earliest_timeout))
            _earliest_timeout = (*outgoing_command)->sent_time();

        // タイムアウトしたらピアを切断する
        if (_earliest_timeout != 0 &&
            (UDP_TIME_DIFFERENCE(service_time, _earliest_timeout) >= _timeout_maximum ||
                ((*outgoing_command)->round_trip_timeout() >= (*outgoing_command)->round_trip_timeout_limit() &&
                    UDP_TIME_DIFFERENCE(service_time, _earliest_timeout) >= _timeout_minimum))) {
            return 1;
        }

        if ((*outgoing_command)->HasPayload()) {
            _reliable_data_in_transit -= (*outgoing_command)->fragment_length();
        }

        net->increase_segments_lost(1);

        (*outgoing_command)->round_trip_timeout((*outgoing_command)->round_trip_timeout() * 2);

        _outgoing_reliable_commands.push_back(*outgoing_command);

        // TODO: ENetの条件式とは違うため、要検証（おそらく意味は同じであるはず）
        if (!_sent_reliable_commands.empty() && _sent_reliable_commands.size() == 1) {
            _next_timeout = (*current_command)->sent_time() + (*current_command)->round_trip_timeout();
        }
    }

    return 0;
}

RUdpProtocolCommand
RUdpCommandPod::RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id,
    std::shared_ptr<RUdpChannel> &channel)
{
    std::shared_ptr<RUdpOutgoingCommand> outgoing_command;
    auto it = _sent_reliable_commands.begin();
    auto it_end = _sent_reliable_commands.end();
    auto no_sent_reliable_command_matched = false;

    for (; it != it_end; ++it)
    {
        if ((*it)->reliable_sequence_number() == reliable_sequence_number &&
            (*it)->header_channel_id() == channel_id)
        {
            outgoing_command = (*it);
            break;
        }

        if (it == _sent_reliable_commands.end())
            no_sent_reliable_command_matched = true;
    }

    auto was_sent = true;

    if (no_sent_reliable_command_matched)
    {
        it = _outgoing_reliable_commands.begin();
        it_end = _outgoing_reliable_commands.end();

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
            _reliable_data_in_transit -= outgoing_command->fragment_length();
    }

    if (_sent_reliable_commands.empty())
        return static_cast<RUdpProtocolCommand>(command_number);

    outgoing_command = _sent_reliable_commands.front();

    _next_timeout = outgoing_command->sent_time() + outgoing_command->round_trip_timeout();

    if (no_sent_reliable_command_matched)
        _outgoing_reliable_commands.erase(it);
    else
        _sent_reliable_commands.erase(it);

    return static_cast<RUdpProtocolCommand>(command_number);
}

void
RUdpCommandPod::remove_sent_unreliable_commands()
{
    while (!_sent_unreliable_commands.empty()) {
        auto &outgoing_command = _sent_unreliable_commands.front();

        if (outgoing_command->HasPayload()) {
            if (outgoing_command->segment().use_count() == 1)
                outgoing_command->segment()->AddFlag(static_cast<uint32_t>(RUdpSegmentFlag::SENT));

            outgoing_command->segment()->Destroy();
        }

        _sent_unreliable_commands.pop_front();
    }
}
