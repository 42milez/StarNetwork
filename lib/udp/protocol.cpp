#include "protocol.h"
#include "udp.h"

size_t
udp_protocol_command_size(uint8_t command_number)
{
    return command_sizes[command_number & PROTOCOL_COMMAND_MASK];
}

UdpProtocol::UdpProtocol() : _recalculate_bandwidth_limits(false)
{}

void
UdpProtocol::_udp_protocol_dispatch_state(const std::shared_ptr<UdpPeer> &peer, const UdpPeerState state)
{
    peer->change_state(state);

    if (!peer->needs_dispatch())
    {
        peer->needs_dispatch(true);

        _dispatch_queue.push(peer);
    }
}

void
UdpProtocol::send_acknowledgements(std::shared_ptr<UdpPeer> &peer)
{
    auto *command = _chamber->command_insert_pos();
    auto *buffer = _chamber->buffer_insert_pos();

    while (peer->acknowledgement_exists())
    {
        // auto ack = peer->acknowledgements.front();
        auto ack = peer->pop_acknowledgement();

        // 送信継続
        // - コマンドバッファに空きがない
        // - データバッファに空きがない
        // - ピアの MTU とパケットサイズの差が UdpProtocolAcknowledge のサイズ未満
        if (!_chamber->command_buffer_have_enough_space() ||
            !_chamber->data_buffer_have_enough_space() ||
            peer->mtu() - _chamber->packet_size() < sizeof(UdpProtocolAcknowledge))
        {
            _chamber->continue_sending(true);

            break;
        }

        buffer->data = command;
        buffer->data_length = sizeof(UdpProtocolAcknowledge);

        _chamber->increase_packet_size(buffer->data_length);

        auto reliable_sequence_number = htons(ack->command.header.reliable_sequence_number);

        command->header.command = PROTOCOL_COMMAND_ACKNOWLEDGE;
        command->header.channel_id = ack->command.header.channel_id;
        command->header.reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_sent_time = htons(ack->sent_time);

        if ((ack->command.header.command & PROTOCOL_COMMAND_MASK) == PROTOCOL_COMMAND_DISCONNECT)
            _udp_protocol_dispatch_state(peer, UdpPeerState::ZOMBIE);

        ++command;
        ++buffer;
    }

    _chamber->update_command_count(command);
    _chamber->update_buffer_count(buffer);
}

void
UdpProtocol::notify_disconnect(const std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event)
{
    if (peer->state_is_ge(UdpPeerState::CONNECTION_PENDING))
        // ピアを切断するのでバンド幅を再計算する
        _recalculate_bandwidth_limits = true;

    // ピアのステートが以下の３つの内のいずれかである場合
    // 1. DISCONNECTED,
    // 2. ACKNOWLEDGING_CONNECT,
    // 3. CONNECTION_PENDING
    //if (peer->state != UdpPeerState::CONNECTING && peer->state < UdpPeerState::CONNECTION_SUCCEEDED)
    if (!peer->state_is(UdpPeerState::CONNECTING) && peer->state_is_lt(UdpPeerState::CONNECTION_SUCCEEDED))
    {
        peer->udp_peer_reset();
    }
        // ピアが接続済みである場合
    else if (event != nullptr)
    {
        event->type = UdpEventType::DISCONNECT;
        event->peer = peer;
        event->data = 0;

        peer->udp_peer_reset();
    }
    else
    {
        peer->event_data(0);

        _udp_protocol_dispatch_state(peer, UdpPeerState::ZOMBIE);
    }
}

bool
UdpProtocol::_udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<UdpPeer> &peer, uint32_t service_time)
{
    /*
    auto *command = &_commands[_command_count];
    auto *buffer = &_buffers[_buffer_count];
    auto window_exceeded = 0;
    auto window_wrap = false;
    auto can_ping = true;
    auto current_command = peer->outgoing_reliable_commands.begin();

    while (current_command != peer->outgoing_reliable_commands.end())
    {
        auto outgoing_command = current_command;

        auto channel = outgoing_command->command->header.channel_id < peer->channels.size() ?
                       peer->channels.at(outgoing_command->command->header.channel_id) :
                       nullptr;

        auto reliable_window = outgoing_command->reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;

        //  check reliable window is available
        // --------------------------------------------------

        if (channel != nullptr)
        {
            if (!window_wrap && window_wraps(channel, reliable_window, outgoing_command))
                window_wrap = true;

            if (window_wrap)
            {
                ++current_command;

                continue;
            }
        }

        //  check packet exceeds window size
        // --------------------------------------------------

        if (outgoing_command->packet != nullptr)
        {
            if (!window_exceeded)
            {
                auto window_size = (peer->packet_throttle * peer->window_size) / PEER_PACKET_THROTTLE_SCALE;

                if (window_exceeds(peer, window_size, outgoing_command))
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

        if (_sending_continues(command, buffer, peer, outgoing_command))
        {
            _continue_sending = true;

            break;
        }

        ++current_command;

        if (channel != nullptr && outgoing_command->send_attempts < 1)
        {
            channel->used_reliable_windows |= 1 << reliable_window;
            ++channel->reliable_windows[reliable_window];
        }

        ++outgoing_command->send_attempts;

        if (outgoing_command->round_trip_timeout == 0)
        {
            outgoing_command->round_trip_timeout = peer->round_trip_time + 4 * peer->round_trip_time_variance;
            outgoing_command->round_trip_timeout_limit = peer->timeout_limit * outgoing_command->round_trip_timeout;
        }

        if (peer->sent_reliable_commands.empty())
            peer->next_timeout = _service_time + outgoing_command->round_trip_timeout;

        outgoing_command->sent_time = _service_time;

        buffer->data = command;
        buffer->data_length = command_sizes[outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK];

        _packet_size += buffer->data_length;
        _header_flags |= PROTOCOL_HEADER_FLAG_SENT_TIME;

        // MEMO: bufferには「コマンド、データ、コマンド、データ・・・」という順番でパケットが挿入される
        //       これは受信側でパケットを正しく識別するための基本

        *command = *outgoing_command->command;

        if (outgoing_command->packet != nullptr)
        {
            ++buffer;

            buffer->data = outgoing_command->packet->data + outgoing_command->fragment_offset;
            buffer->data_length = outgoing_command->fragment_length;

            _packet_size += outgoing_command->fragment_length;

            peer->reliable_data_in_transit += outgoing_command->fragment_length;
        }

        peer->sent_reliable_commands.push_back(*outgoing_command);
        peer->outgoing_reliable_commands.erase(outgoing_command);

        ++peer->packets_sent;

        ++command;
        ++buffer;
    }

    _command_count = command - _commands;
    _buffer_count = buffer - _buffers;

    return can_ping;
    */

    auto can_ping = peer->load_reliable_commands_into_chamber(_chamber, service_time);

    return can_ping;
}

void
UdpProtocol::_udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<UdpPeer> &peer)
{
    /*
    auto *command = _chamber->command_insert_pos();
    auto *buffer = _chamber->buffer_insert_pos();

    auto current_command = peer->outgoing_unreliable_commands.begin();

    while (current_command != peer->outgoing_unreliable_commands.end())
    {
        auto outgoing_command = current_command;
        auto command_size = command_sizes[outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK];

        if (_sending_continues(command, buffer, peer, outgoing_command))
        {
            _chamber->continue_sending(true);

            break;
        }

        ++current_command;

        if (outgoing_command->packet != nullptr && outgoing_command->fragment_offset == 0)
        {
            peer->packet_throttle_counter += PEER_PACKET_THROTTLE_COUNTER;
            peer->packet_throttle_counter %= PEER_PACKET_THROTTLE_SCALE;

            if (peer->packet_throttle_counter > peer->packet_throttle)
            {
                uint16_t reliable_sequence_number = outgoing_command->reliable_sequence_number;
                uint16_t unreliable_sequence_number = outgoing_command->unreliable_sequence_number;

                for (;;)
                {
                    if (current_command == peer->outgoing_unreliable_commands.end())
                        break;

                    outgoing_command = current_command;

                    if (outgoing_command->reliable_sequence_number != reliable_sequence_number ||
                        outgoing_command->unreliable_sequence_number != unreliable_sequence_number)
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

        _packet_size += buffer->data_length;

        *command = *outgoing_command->command;

        peer->outgoing_unreliable_commands.erase(outgoing_command);

        if (outgoing_command->packet != nullptr)
        {
            ++buffer;

            buffer->data = outgoing_command->packet->data + outgoing_command->fragment_offset;
            buffer->data_length = outgoing_command->fragment_length;

            _packet_size += buffer->data_length;

            peer->sent_unreliable_commands.push_back(*outgoing_command);
        }

        ++command;
        ++buffer;
    }

    _command_count = command - _commands;
    _buffer_count = buffer - _buffers;

    if (peer->state == UdpPeerState::DISCONNECT_LATER &&
        peer->outgoing_reliable_commands.empty() &&
        peer->outgoing_unreliable_commands.empty() &&
        peer->sent_reliable_commands.empty())
    {
        udp_peer_disconnect(peer);
    }
    */

    peer->load_unreliable_commands_into_chamber(_chamber, service_time);
}

void
UdpProtocol::_udp_protocol_remove_sent_unreliable_commands(const std::shared_ptr<UdpPeer> &peer)
{
    while (!peer->sent_unreliable_commands.empty())
    {
        auto outgoing_command = peer->sent_unreliable_commands.begin();


        if (outgoing_command->packet != nullptr)
        {
            if (outgoing_command->packet.use_count() == 0)
                outgoing_command->packet->flags |= static_cast<uint32_t >(UdpPacketFlag::SENT);
        }

        peer->sent_unreliable_commands.pop_front();
    }
}

bool
UdpProtocol::recalculate_bandwidth_limits()
{
    return _recalculate_bandwidth_limits;
}

void
UdpProtocol::recalculate_bandwidth_limits(bool val)
{
    _recalculate_bandwidth_limits = val;
}

bool
UdpProtocol::continue_sending()
{
    return _chamber->continue_sending();
}

void
UdpProtocol::continue_sending(bool val)
{
    _chamber->continue_sending(val);
}
