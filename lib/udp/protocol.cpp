#include "protocol.h"
#include "udp.h"

size_t
udp_protocol_command_size(uint8_t command_number)
{
    return command_sizes[command_number & PROTOCOL_COMMAND_MASK];
}

UdpProtocol::UdpProtocol() : _recalculate_bandwidth_limits(false),
                             _bandwidth_limited_peers(0),
                             _bandwidth_throttle_epoch(0),
                             _connected_peers(0)
{}

void
UdpProtocol::_udp_protocol_dispatch_state(std::shared_ptr<UdpPeer> &peer, const UdpPeerState state)
{
    change_state(peer, state);

    if (!peer->needs_dispatch())
    {
        peer->needs_dispatch(true);

        _dispatch_queue->push(peer);
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
        if (!_chamber->command_buffer_have_enough_space(command) ||
            !_chamber->data_buffer_have_enough_space(buffer) ||
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
UdpProtocol::notify_disconnect(std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event)
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
        udp_peer_reset(peer);
    }
        // ピアが接続済みである場合
    else if (event != nullptr)
    {
        event->type = UdpEventType::DISCONNECT;
        event->peer = peer;
        event->data = 0;

        udp_peer_reset(peer);
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
    auto can_ping = peer->load_reliable_commands_into_chamber(_chamber, service_time);

    return can_ping;
}

void
UdpProtocol::_udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<UdpPeer> &peer, uint32_t service_time)
{
    auto can_disconnect = peer->load_unreliable_commands_into_chamber(_chamber);

    if (can_disconnect)
        peer->udp_peer_disconnect();
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

std::unique_ptr<UdpChamber> &
UdpProtocol::chamber()
{
    return _chamber;
}

int
UdpProtocol::dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event)
{
    while (_dispatch_queue->peer_exists())
    {
        auto peer = _dispatch_queue->pop_peer();

        peer->needs_dispatch(false);

        if (peer->state_is(UdpPeerState::CONNECTION_PENDING) ||
            peer->state_is(UdpPeerState::CONNECTION_SUCCEEDED))
        {
            // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
            change_state(peer, UdpPeerState::CONNECTED);

            event->type = UdpEventType::CONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            return 1;
        }
        else if (peer->state_is(UdpPeerState::ZOMBIE))
        {
            recalculate_bandwidth_limits(true);

            event->type = UdpEventType::DISCONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            // ゾンビ状態になったピアはリセットする
            udp_peer_reset(peer);

            return 1;
        }
        else if (peer->state_is(UdpPeerState::CONNECTED))
        {
            if (!peer->dispatched_command_exists())
                continue;

            // 接続済みのピアからはコマンドを受信する
            event->packet = peer->udp_peer_receive(event->channel_id);

            if (event->packet == nullptr)
                continue;

            event->type = UdpEventType::RECEIVE;
            event->peer = peer;

            // ディスパッチすべきピアが残っている場合は、ディスパッチ待ちキューにピアを投入する
            if (peer->dispatched_command_exists())
            {
                peer->needs_dispatch(true);

                _dispatch_queue->push(peer);
            }

            return 1;
        }
    }

    return 0;
}

void
UdpProtocol::udp_peer_reset(const std::shared_ptr<UdpPeer> &peer)
{
    disconnect(peer);

    peer->reset();

    udp_peer_reset_queues(peer);
}

void
UdpProtocol::udp_peer_reset_queues(const std::shared_ptr<UdpPeer> &peer)
{
    std::unique_ptr<UdpChannel> channel;

    if (peer->needs_dispatch())
        peer->needs_dispatch(false);

    if (peer->acknowledgement_exists())
        peer->clear_acknowledgement();

    peer->command()->clear_sent_reliable_command();
    peer->command()->clear_sent_unreliable_command();

    peer->command()->clear_outgoing_reliable_command();
    peer->command()->clear_outgoing_unreliable_command();

    while (peer->dispatched_command_exists())
        peer->clear_dispatched_command();

    if (peer->channel_exists())
        peer->clear_channel();
}

void
UdpProtocol::increase_connected_peers()
{
    ++_connected_peers;
}

void
UdpProtocol::decrease_connected_peers()
{
    --_connected_peers;
}

void
UdpProtocol::increase_bandwidth_limited_peers()
{
    ++_bandwidth_limited_peers;
}

void
UdpProtocol::decrease_bandwidth_limited_peers()
{
    --_bandwidth_limited_peers;
}

void
UdpProtocol::change_state(const std::shared_ptr<UdpPeer> &peer, const UdpPeerState &state)
{
    if (state == UdpPeerState::CONNECTED || state == UdpPeerState::DISCONNECT_LATER)
        connect(peer);
    else
        disconnect(peer);

    peer->net()->state(state);
}

void
UdpProtocol::connect(const std::shared_ptr<UdpPeer> &peer)
{
    if (!peer->net()->state_is(UdpPeerState::CONNECTED) && !peer->net()->state_is(UdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            increase_bandwidth_limited_peers();

        increase_connected_peers();
    }
}

void
UdpProtocol::disconnect(const std::shared_ptr<UdpPeer> &peer)
{
    if (peer->net()->state_is(UdpPeerState::CONNECTED) || peer->net()->state_is(UdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            decrease_bandwidth_limited_peers();

        decrease_connected_peers();
    }
}

size_t
UdpProtocol::connected_peers()
{
    return _connected_peers;
}

uint32_t
UdpProtocol::bandwidth_throttle_epoch()
{
    return _bandwidth_throttle_epoch;
}

void
UdpProtocol::bandwidth_throttle_epoch(uint32_t val)
{
    _bandwidth_throttle_epoch = val;
}

size_t
UdpProtocol::bandwidth_limited_peers()
{
    return _bandwidth_limited_peers;
}
