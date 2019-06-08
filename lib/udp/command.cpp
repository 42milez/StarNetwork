#include "command.h"
#include "udp.h"

namespace
{
    bool
    window_wraps(const std::shared_ptr<UdpChannel> &channel,
                 int reliable_window,
                 const std::shared_ptr<UdpOutgoingCommand> &outgoing_command)
    {
        auto has_not_sent_once = outgoing_command->send_attempts == 0;

        auto first_command_in_window = !(outgoing_command->reliable_sequence_number % PEER_RELIABLE_WINDOW_SIZE);

        auto all_available_windows_are_in_use = channel->reliable_windows.at(
            (reliable_window + PEER_RELIABLE_WINDOWS - 1) % PEER_RELIABLE_WINDOWS
        ) >= PEER_RELIABLE_WINDOW_SIZE;

        auto existing_commands_are_in_flight = channel->used_reliable_windows & (
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
                   const std::shared_ptr<UdpOutgoingCommand> &outgoing_command)
    {
        return (reliable_data_in_transit + outgoing_command->fragment_length) > std::max(window_size, mtu);
    }
}

UdpOutgoingCommand::UdpOutgoingCommand() : reliable_sequence_number(0),
                                           unreliable_sequence_number(0),
                                           sent_time(0),
                                           round_trip_timeout(0),
                                           round_trip_timeout_limit(0),
                                           fragment_offset(0),
                                           fragment_length(0),
                                           send_attempts(0)
{}

UdpCommandPod::UdpCommandPod() :
    _incoming_data_total(0),
    _outgoing_data_total(0),
    _outgoing_reliable_sequence_number(0),
    _incoming_unsequenced_group(0),
    _outgoing_unsequenced_group(0),
    _round_trip_time(0),
    _round_trip_time_variance(0),
    _timeout_limit(0),
    _next_timeout(0)
{}

void
UdpCommandPod::setup_outgoing_command(std::shared_ptr<UdpOutgoingCommand> &outgoing_command)
{
    UdpChannel channel;

    _outgoing_data_total +=
        udp_protocol_command_size(outgoing_command->command->header.command) + outgoing_command->fragment_length;

    if (outgoing_command->command->header.channel_id == 0xFF)
    {
        ++_outgoing_reliable_sequence_number;

        outgoing_command->reliable_sequence_number = _outgoing_reliable_sequence_number;
        outgoing_command->unreliable_sequence_number = 0;
    }
    else if (outgoing_command->command->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
    {
        ++channel.outgoing_reliable_sequence_number;
        channel.outgoing_unreliable_seaquence_number = 0;

        outgoing_command->reliable_sequence_number = channel.outgoing_reliable_sequence_number;
        outgoing_command->unreliable_sequence_number = 0;
    }
    else if (outgoing_command->command->header.command & PROTOCOL_COMMAND_FLAG_UNSEQUENCED)
    {
        ++_outgoing_unsequenced_group;

        outgoing_command->reliable_sequence_number = 0;
        outgoing_command->unreliable_sequence_number = 0;
    }
    else
    {
        if (outgoing_command->fragment_offset == 0)
            ++channel.outgoing_unreliable_seaquence_number;

        outgoing_command->reliable_sequence_number = channel.outgoing_reliable_sequence_number;
        outgoing_command->unreliable_sequence_number = channel.outgoing_unreliable_seaquence_number;
    }

    outgoing_command->send_attempts = 0;
    outgoing_command->sent_time = 0;
    outgoing_command->round_trip_timeout = 0;
    outgoing_command->round_trip_timeout_limit = 0;
    outgoing_command->command->header.reliable_sequence_number = htons(outgoing_command->reliable_sequence_number);

    auto cmd = outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK;

    if (cmd == PROTOCOL_COMMAND_SEND_UNRELIABLE)
    {
        outgoing_command->command->send_unreliable.unreliable_sequence_number = htons(outgoing_command->unreliable_sequence_number);
    }
    else if (cmd == PROTOCOL_COMMAND_SEND_UNSEQUENCED)
    {
        outgoing_command->command->send_unsequenced.unsequenced_group = htons(_outgoing_unsequenced_group);
    }

    if (outgoing_command->command->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
        _outgoing_reliable_commands.push_back(outgoing_command);
    else
        _outgoing_unreliable_commands.push_back(outgoing_command);
}

void
UdpCommandPod::push_outgoing_reliable_command(std::shared_ptr<UdpOutgoingCommand> &command)
{
    _outgoing_reliable_commands.push_front(command);
}

bool
UdpCommandPod::load_reliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber,
                                          uint32_t mtu,
                                          uint32_t packet_throttle,
                                          uint32_t window_size,
                                          uint32_t service_time)
{
    auto *command = chamber->command_insert_pos();
    auto *buffer = chamber->buffer_insert_pos();
    auto window_exceeded = 0;
    auto window_wrap = false;
    auto can_ping = true;
    auto current_command = _outgoing_reliable_commands.begin();

    while (current_command != _outgoing_reliable_commands.end())
    {
        auto outgoing_command = current_command;

        auto channel = (*outgoing_command)->command->header.channel_id < _channels.size() ?
                       _channels.at((*outgoing_command)->command->header.channel_id) :
                       nullptr;

        auto reliable_window = (*outgoing_command)->reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;

        //  check reliable window is available
        // --------------------------------------------------

        if (channel != nullptr)
        {
            if (!window_wrap && window_wraps(channel, reliable_window, (*outgoing_command)))
                window_wrap = true;

            if (window_wrap)
            {
                ++current_command;

                continue;
            }
        }

        //  check packet exceeds window size
        // --------------------------------------------------

        if ((*outgoing_command)->packet != nullptr)
        {
            if (!window_exceeded)
            {
                auto ws = (packet_throttle * window_size) / PEER_PACKET_THROTTLE_SCALE;

                if (window_exceeds(chamber->reliable_data_in_transit(), mtu, ws, (*outgoing_command)))
                    window_exceeded = true;
            }

            if (window_exceeded)
            {
                ++current_command;

                continue;
            }
        }

        //
        // --------------------------------------------------

        can_ping = false;

        if (chamber->sending_continues(command, buffer, mtu, (*outgoing_command)))
        {
            chamber->continue_sending(true);

            break;
        }

        ++current_command;

        if (channel != nullptr && (*outgoing_command)->send_attempts < 1)
        {
            channel->used_reliable_windows |= 1 << reliable_window;
            ++channel->reliable_windows[reliable_window];
        }

        ++(*outgoing_command)->send_attempts;

        if ((*outgoing_command)->round_trip_timeout == 0)
        {
            (*outgoing_command)->round_trip_timeout = _round_trip_time + 4 * _round_trip_time_variance;
            (*outgoing_command)->round_trip_timeout_limit = _timeout_limit * (*outgoing_command)->round_trip_timeout;
        }

        if (!chamber->sent_reliable_command_exists())
            _next_timeout = service_time + (*outgoing_command)->round_trip_timeout;

        (*outgoing_command)->sent_time = service_time;

        buffer->data = command;
        buffer->data_length = command_sizes[(*outgoing_command)->command->header.command & PROTOCOL_COMMAND_MASK];

        chamber->increase_packet_size(buffer->data_length);

        auto flags = chamber->header_flags();
        chamber->header_flags(flags | PROTOCOL_HEADER_FLAG_SENT_TIME);

        // MEMO: bufferには「コマンド、データ、コマンド、データ・・・」という順番でパケットが挿入される
        //       これは受信側でパケットを正しく識別するための基本

        *command = *(*outgoing_command)->command;

        if ((*outgoing_command)->packet != nullptr)
        {
            ++buffer;

            buffer->data = (*outgoing_command)->packet->data + (*outgoing_command)->fragment_offset;
            buffer->data_length = (*outgoing_command)->fragment_length;

            chamber->increase_packet_size((*outgoing_command)->fragment_length);
            chamber->increse_reliable_data_in_transit((*outgoing_command)->fragment_length);
        }

        chamber->push_sent_reliable_command(*outgoing_command);

        _outgoing_reliable_commands.erase(outgoing_command);

        // MEMO: push_sent_reliable_command() でインクリメント
        //++_packets_sent;

        ++command;
        ++buffer;
    }

    chamber->update_command_count(command);
    chamber->update_buffer_count(buffer);

    return can_ping;
}

bool
UdpCommandPod::load_unreliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber,
                                                     uint32_t mtu,
                                                     uint32_t packet_throttle,
                                                     uint32_t window_size,
                                                     uint32_t service_time)
{
    auto *command = chamber->command_insert_pos();
    auto *buffer = chamber->buffer_insert_pos();
    auto can_disconnect = false;
    auto current_command = _outgoing_unreliable_commands.begin();

    while (current_command != _outgoing_unreliable_commands.end())
    {
        auto outgoing_command = current_command;
        auto command_size = command_sizes[(*outgoing_command)->command->header.command & PROTOCOL_COMMAND_MASK];

        if (chamber->sending_continues(command, buffer, mtu, (*outgoing_command)))
        {
            chamber->continue_sending(true);

            break;
        }

        ++current_command;

        if ((*outgoing_command)->packet != nullptr && (*outgoing_command)->fragment_offset == 0)
        {
            peer->packet_throttle_counter += PEER_PACKET_THROTTLE_COUNTER;
            peer->packet_throttle_counter %= PEER_PACKET_THROTTLE_SCALE;

            if (peer->packet_throttle_counter > peer->packet_throttle)
            {
                uint16_t reliable_sequence_number = (*outgoing_command)->reliable_sequence_number;
                uint16_t unreliable_sequence_number = (*outgoing_command)->unreliable_sequence_number;

                for (;;)
                {
                    if (current_command == _outgoing_unreliable_commands.end())
                        break;

                    outgoing_command = current_command;

                    if ((*outgoing_command)->reliable_sequence_number != reliable_sequence_number ||
                        (*outgoing_command)->unreliable_sequence_number != unreliable_sequence_number)
                    {
                        break;
                    }

                    ++current_command;
                }

                continue;
            }
        }

        buffer->data = command;
        buffer->data_length = command_size;

        chamber->increase_packet_size(buffer->data_length);

        *command = *(*outgoing_command)->command;

        _outgoing_unreliable_commands.erase(outgoing_command);

        if ((*outgoing_command)->packet != nullptr)
        {
            ++buffer;

            buffer->data = (*outgoing_command)->packet->data + (*outgoing_command)->fragment_offset;
            buffer->data_length = (*outgoing_command)->fragment_length;

            chamber->increase_packet_size(buffer->data_length);

            chamber->push_sent_unreliable_command(*outgoing_command);
        }

        ++command;
        ++buffer;
    }

    chamber->update_command_count(command);
    chamber->update_buffer_count(buffer);

    // TODO: stateやthrottle関連のプロパティは新しいクラスにまとめたい（このクラスはUdpPeerが所有する）
    if (peer->state_is(UdpPeerState::DISCONNECT_LATER) &&
        _outgoing_reliable_commands.empty() &&
        _outgoing_unreliable_commands.empty() &&
        !chamber->sent_reliable_command_exists())
    {
        peer->udp_peer_disconnect();
    }

    // ↑のロジックの結果次第でtrue/falseを返す
    // udp_peer_disconnect() はUdpPeerPodから呼ぶ
    return can_disconnect;
}

uint32_t
UdpCommandPod::outgoing_data_total()
{
    return _outgoing_data_total;
}

void
UdpCommandPod::outgoing_data_total(uint32_t val)
{
    _outgoing_data_total = val;
}

