#include "RUdpChannel.h"
#include "RUdpCommandSize.h"
#include "RUdpCommon.h"
#include "RUdpProtocol.h"

#define IS_PEER_NOT_CONNECTED(peer) \
    !peer->state_is(RUdpPeerState::CONNECTED) && peer->state_is(RUdpPeerState::DISCONNECT_LATER)

RUdpProtocol::RUdpProtocol() : _recalculate_bandwidth_limits(false),
                             _bandwidth_limited_peers(0),
                             _bandwidth_throttle_epoch(0),
                             _connected_peers(0)
{}

void
RUdpProtocol::_udp_protocol_dispatch_state(std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState state)
{
    change_state(peer, state);

    if (!peer->needs_dispatch())
    {
        peer->needs_dispatch(true);

        _dispatch_queue->push(peer);
    }
}

void
RUdpProtocol::send_acknowledgements(std::shared_ptr<RUdpPeer> &peer)
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
        // - ピアの MTU とパケットサイズの差が RUdpProtocolAcknowledge のサイズ未満
        if (!_chamber->command_buffer_have_enough_space(command) ||
            !_chamber->data_buffer_have_enough_space(buffer) ||
            peer->mtu() - _chamber->packet_size() < sizeof(RUdpProtocolAcknowledge))
        {
            _chamber->continue_sending(true);

            break;
        }

        buffer->data = command;
        buffer->data_length = sizeof(RUdpProtocolAcknowledge);

        _chamber->increase_packet_size(buffer->data_length);

        auto reliable_sequence_number = htons(ack->command.header.reliable_sequence_number);

        command->header.command = PROTOCOL_COMMAND_ACKNOWLEDGE;
        command->header.channel_id = ack->command.header.channel_id;
        command->header.reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_sent_time = htons(ack->sent_time);

        if ((ack->command.header.command & PROTOCOL_COMMAND_MASK) == PROTOCOL_COMMAND_DISCONNECT)
            _udp_protocol_dispatch_state(peer, RUdpPeerState::ZOMBIE);

        ++command;
        ++buffer;
    }

    _chamber->update_command_count(command);
    _chamber->update_buffer_count(buffer);
}

void
RUdpProtocol::notify_disconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event)
{
    if (peer->state_is_ge(RUdpPeerState::CONNECTION_PENDING))
        // ピアを切断するのでバンド幅を再計算する
        _recalculate_bandwidth_limits = true;

    // ピアのステートが以下の３つの内のいずれかである場合
    // 1. DISCONNECTED,
    // 2. ACKNOWLEDGING_CONNECT,
    // 3. CONNECTION_PENDING
    //if (peer->state != RUdpPeerState::CONNECTING && peer->state < RUdpPeerState::CONNECTION_SUCCEEDED)
    if (!peer->state_is(RUdpPeerState::CONNECTING) && peer->state_is_lt(RUdpPeerState::CONNECTION_SUCCEEDED))
    {
        udp_peer_reset(peer);
    }
        // ピアが接続済みである場合
    else if (event != nullptr)
    {
        event->type = RUdpEventType::DISCONNECT;
        event->peer = peer;
        event->data = 0;

        udp_peer_reset(peer);
    }
    else
    {
        peer->event_data(0);

        _udp_protocol_dispatch_state(peer, RUdpPeerState::ZOMBIE);
    }
}

bool
RUdpProtocol::_udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time)
{
    auto can_ping = peer->load_reliable_commands_into_chamber(_chamber, service_time);

    return can_ping;
}

void
RUdpProtocol::_udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time)
{
    auto can_disconnect = peer->load_unreliable_commands_into_chamber(_chamber);

    if (can_disconnect)
        peer->udp_peer_disconnect();
}

bool
RUdpProtocol::recalculate_bandwidth_limits()
{
    return _recalculate_bandwidth_limits;
}

void
RUdpProtocol::recalculate_bandwidth_limits(bool val)
{
    _recalculate_bandwidth_limits = val;
}

bool
RUdpProtocol::continue_sending()
{
    return _chamber->continue_sending();
}

void
RUdpProtocol::continue_sending(bool val)
{
    _chamber->continue_sending(val);
}

std::unique_ptr<UdpChamber> &
RUdpProtocol::chamber()
{
    return _chamber;
}

int
RUdpProtocol::dispatch_incoming_commands(std::unique_ptr<RUdpEvent> &event)
{
    while (_dispatch_queue->peer_exists())
    {
        auto peer = _dispatch_queue->pop_peer();

        peer->needs_dispatch(false);

        if (peer->state_is(RUdpPeerState::CONNECTION_PENDING) ||
            peer->state_is(RUdpPeerState::CONNECTION_SUCCEEDED))
        {
            // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
            change_state(peer, RUdpPeerState::CONNECTED);

            event->type = RUdpEventType::CONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            return 1;
        }
        else if (peer->state_is(RUdpPeerState::ZOMBIE))
        {
            recalculate_bandwidth_limits(true);

            event->type = RUdpEventType::DISCONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            // ゾンビ状態になったピアはリセットする
            udp_peer_reset(peer);

            return 1;
        }
        else if (peer->state_is(RUdpPeerState::CONNECTED))
        {
            if (!peer->dispatched_command_exists())
                continue;

            // 接続済みのピアからはコマンドを受信する
            event->packet = peer->udp_peer_receive(event->channel_id);

            if (event->packet == nullptr)
                continue;

            event->type = RUdpEventType::RECEIVE;
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
RUdpProtocol::udp_peer_reset(const std::shared_ptr<RUdpPeer> &peer)
{
    disconnect(peer);

    peer->reset();

    udp_peer_reset_queues(peer);
}

void
RUdpProtocol::udp_peer_reset_queues(const std::shared_ptr<RUdpPeer> &peer)
{
    std::unique_ptr<RUdpChannel> channel;

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
RUdpProtocol::increase_connected_peers()
{
    ++_connected_peers;
}

void
RUdpProtocol::decrease_connected_peers()
{
    --_connected_peers;
}

void
RUdpProtocol::increase_bandwidth_limited_peers()
{
    ++_bandwidth_limited_peers;
}

void
RUdpProtocol::decrease_bandwidth_limited_peers()
{
    --_bandwidth_limited_peers;
}

void
RUdpProtocol::change_state(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state)
{
    if (state == RUdpPeerState::CONNECTED || state == RUdpPeerState::DISCONNECT_LATER)
        connect(peer);
    else
        disconnect(peer);

    peer->net()->state(state);
}

void
RUdpProtocol::connect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (!peer->net()->state_is(RUdpPeerState::CONNECTED) && !peer->net()->state_is(RUdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            increase_bandwidth_limited_peers();

        increase_connected_peers();
    }
}

void
RUdpProtocol::disconnect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (peer->net()->state_is(RUdpPeerState::CONNECTED) || peer->net()->state_is(RUdpPeerState::DISCONNECT_LATER))
    {
        if (peer->net()->incoming_bandwidth() != 0)
            decrease_bandwidth_limited_peers();

        decrease_connected_peers();
    }
}

size_t
RUdpProtocol::connected_peers()
{
    return _connected_peers;
}

uint32_t
RUdpProtocol::bandwidth_throttle_epoch()
{
    return _bandwidth_throttle_epoch;
}

void
RUdpProtocol::bandwidth_throttle_epoch(uint32_t val)
{
    _bandwidth_throttle_epoch = val;
}

size_t
RUdpProtocol::bandwidth_limited_peers()
{
    return _bandwidth_limited_peers;
}

void
RUdpProtocol::bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth, const std::vector<std::shared_ptr<RUdpPeer>> &peers)
{
    if (UDP_TIME_DIFFERENCE(service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
    {
        auto time_current = udp_time_get();
        auto time_elapsed = time_current - _bandwidth_throttle_epoch;
        auto peers_remaining = _connected_peers;
        auto data_total = ~0u;
        auto bandwidth = ~0u;
        auto throttle = 0;
        auto bandwidth_limit = 0;
        auto needs_adjustment = _bandwidth_limited_peers > 0 ? true : false;

        if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
            return;

        _bandwidth_throttle_epoch = time_current;

        if (peers_remaining == 0)
            return;

        //  Throttle outgoing bandwidth
        // --------------------------------------------------

        if (outgoing_bandwidth != 0)
        {
            data_total = 0;
            bandwidth = outgoing_bandwidth * (time_elapsed / 1000);

            for (const auto &peer : peers)
            {
                if (IS_PEER_NOT_CONNECTED(peer))
                    continue;

                data_total += peer->outgoing_data_total();
            }
        }

        //  Throttle peer bandwidth : Case A ( adjustment is needed )
        // --------------------------------------------------

        while (peers_remaining > 0 && needs_adjustment)
        {
            needs_adjustment = false;

            if (data_total <= bandwidth)
                throttle = PEER_PACKET_THROTTLE_SCALE;
            else
                throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

            for (auto &peer : peers)
            {
                uint32_t peer_bandwidth;

                if ((IS_PEER_NOT_CONNECTED(peer)) ||
                    peer->incoming_bandwidth() == 0 ||
                    peer->outgoing_bandwidth_throttle_epoch() == time_current)
                {
                    continue;
                }

                peer_bandwidth = peer->incoming_bandwidth() * (time_elapsed / 1000);
                if ((throttle * peer->outgoing_data_total()) / PEER_PACKET_THROTTLE_SCALE <= peer_bandwidth)
                    continue;

                peer->packet_throttle_limit((peer_bandwidth * PEER_PACKET_THROTTLE_SCALE) / peer->outgoing_data_total());

                if (peer->packet_throttle_limit() == 0)
                    peer->packet_throttle_limit(1);

                if (peer->packet_throttle() > peer->packet_throttle_limit())
                    peer->packet_throttle(peer->packet_throttle_limit());

                peer->outgoing_bandwidth_throttle_epoch(time_current);
                peer->incoming_data_total(0);
                peer->outgoing_data_total(0);

                needs_adjustment = true;

                --peers_remaining;

                bandwidth -= peer_bandwidth;
                data_total -= peer_bandwidth;
            }
        }

        //  Throttle peer bandwidth : Case B ( adjustment is NOT needed )
        // --------------------------------------------------

        if (peers_remaining > 0)
        {
            if (data_total <= bandwidth)
                throttle = PEER_PACKET_THROTTLE_SCALE;
            else
                throttle = (bandwidth * PEER_PACKET_THROTTLE_SCALE) / data_total;

            for (auto &peer : peers)
            {
                if ((IS_PEER_NOT_CONNECTED(peer)) || peer->net()->outgoing_bandwidth_throttle_epoch() == time_current)
                    continue;

                peer->net()->packet_throttle_limit(throttle);

                if (peer->net()->packet_throttle() > peer->net()->packet_throttle_limit())
                    peer->net()->packet_throttle(peer->net()->packet_throttle_limit());

                peer->command()->incoming_data_total(0);
                peer->command()->outgoing_data_total(0);
            }
        }

        //  Recalculate Bandwidth Limits
        // --------------------------------------------------

        if (_recalculate_bandwidth_limits)
        {
            _recalculate_bandwidth_limits = false;
            peers_remaining = _connected_peers;
            bandwidth = incoming_bandwidth;
            needs_adjustment = true;

            if (bandwidth == 0)
            {
                bandwidth_limit = 0;
            }
            else
            {
                while (peers_remaining > 0 && needs_adjustment)
                {
                    needs_adjustment = false;
                    bandwidth_limit = bandwidth / peers_remaining;

                    for (auto &peer: peers)
                    {
                        if ((IS_PEER_NOT_CONNECTED(peer)) ||
                            peer->net()->incoming_bandwidth_throttle_epoch() == time_current)
                            continue;

                        if (peer->net()->outgoing_bandwidth() > 0 && peer->net()->outgoing_bandwidth() >= bandwidth_limit)
                            continue;

                        peer->net()->incoming_bandwidth_throttle_epoch(time_current);

                        needs_adjustment = true;

                        --peers_remaining;

                        bandwidth -= peer->net()->outgoing_bandwidth();
                    }
                }
            }

            std::shared_ptr<RUdpProtocolType> cmd;

            for (auto &peer : peers)
            {
                if (IS_PEER_NOT_CONNECTED(peer))
                    continue;

                cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
                cmd->header.channel_id = 0xFF;
                cmd->bandwidth_limit.outgoing_bandwidth = htonl(outgoing_bandwidth);

                if (peer->net()->incoming_bandwidth_throttle_epoch() == time_current)
                    cmd->bandwidth_limit.incoming_bandwidth = htonl(peer->net()->outgoing_bandwidth());
                else
                    cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

                peer->queue_outgoing_command(cmd, nullptr, 0, 0);
            }
        }
    }
}
