#include "core/hash.h"

#include "udp.h"

#define IS_PEER_CONNECTED(peer) \
    peer->state != UdpPeerState::CONNECTED && peer->state != UdpPeerState::DISCONNECT_LATER

void
UdpHost::udp_host_compress(std::shared_ptr<UdpHost> &host)
{
    if (_compressor->destroy != nullptr)
        _compressor->destroy();
}

void
UdpHost::udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor)
{
    if (compressor)
        _compressor = compressor;
}

Error
UdpHost::udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto current_peer = _peers.begin();

    for (; current_peer != _peers.end(); ++current_peer)
    {
        if ((*current_peer)->state == UdpPeerState::DISCONNECTED)
            break;
    }

    if (current_peer == _peers.end())
        return Error::CANT_CREATE;

    (*current_peer)->channels = std::move(std::vector<UdpChannel>(static_cast<int>(channel_count)));

    if ((*current_peer)->channels.empty())
        return Error::CANT_CREATE;

    (*current_peer)->state = UdpPeerState::CONNECTING;
    (*current_peer)->address = address;
    (*current_peer)->connect_id = hash32();

    if (_outgoing_bandwidth == 0)
        (*current_peer)->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
        (*current_peer)->window_size = (_outgoing_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

    if ((*current_peer)->window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        (*current_peer)->window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if ((*current_peer)->window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        (*current_peer)->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    std::shared_ptr<UdpProtocol> cmd = std::make_shared<UdpProtocol>();

    cmd->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    cmd->connect.outgoing_peer_id = htons((*current_peer)->incoming_peer_id);
    cmd->connect.incoming_session_id = (*current_peer)->incoming_session_id;
    cmd->connect.outgoing_session_id = (*current_peer)->outgoing_session_id;
    cmd->connect.mtu = htonl((*current_peer)->mtu);
    cmd->connect.window_size = htonl((*current_peer)->window_size);
    cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    cmd->connect.incoming_bandwidth = htonl(_incoming_bandwidth);
    cmd->connect.outgoing_bandwidth = htonl(_outgoing_bandwidth);
    cmd->connect.packet_throttle_interval = htonl((*current_peer)->packet_throttle_interval);
    cmd->connect.packet_throttle_acceleration = htonl((*current_peer)->packet_throttle_acceleration);
    cmd->connect.packet_throttle_deceleration = htonl((*current_peer)->packet_throttle_deceleration);
    cmd->connect.data = data;

    udp_peer_queue_outgoing_command(*current_peer, cmd, nullptr, 0, 0);

    return Error::OK;
}

void
UdpHost::_udp_host_bandwidth_throttle()
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

    if (_outgoing_bandwidth != 0)
    {
        data_total = 0;
        bandwidth = _outgoing_bandwidth * (time_elapsed / 1000);

        for (const auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            data_total += peer->outgoing_data_total;
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

        for (auto &peer : _peers)
        {
            uint32_t peer_bandwidth;

            if ((IS_PEER_CONNECTED(peer)) ||
                peer->incoming_bandwidth == 0 ||
                peer->outgoing_bandwidth_throttle_epoch == time_current)
            {
                continue;
            }

            peer_bandwidth = peer->incoming_bandwidth * (time_elapsed / 1000);
            if ((throttle * peer->outgoing_data_total) / PEER_PACKET_THROTTLE_SCALE <= peer_bandwidth)
                continue;

            peer->packet_throttle_limit = (peer_bandwidth * PEER_PACKET_THROTTLE_SCALE) / peer->outgoing_data_total;

            if (peer->packet_throttle_limit == 0)
                peer->packet_throttle_limit = 1;

            if (peer->packet_throttle > peer->packet_throttle_limit)
                peer->packet_throttle = peer->packet_throttle_limit;

            peer->outgoing_bandwidth_throttle_epoch = time_current;
            peer->incoming_data_total = 0;
            peer->outgoing_data_total = 0;

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

        for (auto &peer : _peers)
        {
            if ((IS_PEER_CONNECTED(peer)) || peer->outgoing_bandwidth_throttle_epoch == time_current)
                continue;

            peer->packet_throttle_limit = throttle;

            if (peer->packet_throttle > peer->packet_throttle_limit)
                peer->packet_throttle = peer->packet_throttle_limit;

            peer->incoming_data_total = 0;
            peer->outgoing_data_total = 0;
        }
    }

    //  Recalculate Bandwidth Limits
    // --------------------------------------------------

    if (_recalculate_bandwidth_limits)
    {
        _recalculate_bandwidth_limits = false;
        peers_remaining = _connected_peers;
        bandwidth = _incoming_bandwidth;
        needs_adjustment = true;

        if (bandwidth == 0)
            bandwidth_limit = 0;
        else
            while (peers_remaining > 0 && needs_adjustment)
            {
                needs_adjustment = false;
                bandwidth_limit = bandwidth / peers_remaining;

                for (auto &peer: _peers)
                {
                    if ((IS_PEER_CONNECTED(peer)) || peer->incoming_bandwidth_throttle_epoch == time_current)
                        continue;

                    if (peer->outgoing_bandwidth > 0 && peer->outgoing_bandwidth >= bandwidth_limit)
                        continue;

                    peer->incoming_bandwidth_throttle_epoch = time_current;

                    needs_adjustment = true;

                    --peers_remaining;

                    bandwidth -= peer->outgoing_bandwidth;
                }
            }


        std::shared_ptr<UdpProtocol> cmd;

        for (auto &peer : _peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
            cmd->header.channel_id = 0xFF;
            cmd->bandwidth_limit.outgoing_bandwidth = htonl(_outgoing_bandwidth);

            if (peer->incoming_bandwidth_throttle_epoch == time_current)
                cmd->bandwidth_limit.incoming_bandwidth = htonl(peer->outgoing_bandwidth);
            else
                cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

            udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
        }
    }
}

void
UdpHost::_udp_protocol_change_state(std::shared_ptr<UdpPeer> &peer, const UdpPeerState state)
{
    if (state == UdpPeerState::CONNECTED || state == UdpPeerState::DISCONNECT_LATER)
        udp_peer_on_connect(peer);
    else
        udp_peer_on_disconnect(peer);

    peer->state = state;
}

std::shared_ptr<UdpPeer>
UdpHost::_dequeue()
{
    auto ret = _dispatch_queue.front();

    _dispatch_queue.pop();

    return ret;
}

int
UdpHost::_udp_protocol_dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event)
{
    while (!_dispatch_queue.empty())
    {
        auto peer = _dequeue();

        peer->needs_dispatch = false;

        if (peer->state == UdpPeerState::CONNECTION_PENDING ||
            peer->state == UdpPeerState::CONNECTION_SUCCEEDED)
        {
            // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
            _udp_protocol_change_state(peer, UdpPeerState::CONNECTED);

            event->type = UdpEventType::CONNECT;
            event->peer = peer;
            event->data = peer->event_data;

            return 1;
        }
        else if (peer->state == UdpPeerState::ZOMBIE)
        {
            _recalculate_bandwidth_limits = true;

            event->type = UdpEventType::DISCONNECT;
            event->peer = peer;
            event->data = peer->event_data;

            // ゾンビ状態になったピアはリセットする
            udp_peer_reset(peer);

            return 1;
        }
        else if (peer->state == UdpPeerState::CONNECTED)
        {
            if (peer->dispatched_commands.empty())
                continue;

            // 接続済みのピアからはコマンドを受信する
            event->packet = udp_peer_receive(peer, event->channel_id);

            if (event->packet == nullptr)
                continue;

            event->type = UdpEventType::RECEIVE;
            event->peer = peer;

            // ディスパッチすべきピアが残っている場合は、ディスパッチ待ちキューにピアを投入する
            if (!peer->dispatched_commands.empty())
            {
                peer->needs_dispatch = true;

                _dispatch_queue.push(peer);
            }

            return 1;
        }
    }

    return 0;
}

void
UdpHost::_udp_protocol_dispatch_state(std::shared_ptr<UdpPeer> &peer, const UdpPeerState state)
{
    _udp_protocol_change_state(peer, state);

    if (!peer->needs_dispatch)
    {
        peer->needs_dispatch = true;

        _dispatch_queue.push(peer);
    }
}

void
UdpHost::_udp_protocol_send_acknowledgements(std::shared_ptr<UdpPeer> &peer)
{
    auto *command = &_commands[_command_count];
    auto *buffer = &_buffers[_buffer_count];

    for (auto &ack : peer->acknowledgements)
    {
        // 送信継続
        // - コマンドバッファに空きがない
        // - データバッファに空きがない
        // - ピアの MTU とパケットサイズの差が UdpProtocolAcknowledge のサイズ未満
        if (command >= &_commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS] ||
            buffer >= &_buffers[BUFFER_MAXIMUM] ||
            peer->mtu - _packet_size < sizeof(UdpProtocolAcknowledge))
        {
            _continue_sending = true;

            break;
        }

        buffer->data = command;
        buffer->data_length = sizeof(UdpProtocolAcknowledge);

        _packet_size += buffer->data_length;

        auto reliable_sequence_number = htons(ack.command.header.reliable_sequence_number);

        command->header.command = PROTOCOL_COMMAND_ACKNOWLEDGE;
        command->header.channel_id = ack.command.header.channel_id;
        command->header.reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_reliable_sequence_number = reliable_sequence_number;
        command->acknowledge.received_sent_time = htons(ack.sent_time);

        if ((ack.command.header.command & PROTOCOL_COMMAND_MASK) == PROTOCOL_COMMAND_DISCONNECT)
            _udp_protocol_dispatch_state(peer, UdpPeerState::ZOMBIE);

        ++command;
        ++buffer;
    }

    _command_count = command - _commands;
    _buffer_count = buffer - _buffers;
}

int
UdpHost::_udp_protocol_check_timeouts(const std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event)
{
    // ...

    return 0;
}

bool
UdpHost::_udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<UdpPeer> &peer)
{
    // ...

    return true;
}

void
UdpHost::_udp_protocol_send_unreliable_outgoing_commands(const std::shared_ptr<UdpPeer> &peer)
{
    // ...
}

void
UdpHost::_udp_protocol_remove_sent_unreliable_commands(const std::shared_ptr<UdpPeer> &peer)
{
    // ...
}

int
UdpHost::_udp_socket_send(const UdpAddress &address)
{
    // ...

    return 0;
}

int
UdpHost::_protocol_send_outgoing_commands(std::unique_ptr<UdpEvent> &event, bool check_for_timeouts)
{
    uint8_t header_data[sizeof(UdpProtocolHeader) + sizeof(uint32_t)];
    auto *header = reinterpret_cast<UdpProtocolHeader *>(header_data);

    _continue_sending = true;

    while (_continue_sending)
    {
        _continue_sending = false;

        for (auto &peer : _peers)
        {
            if (peer->state == UdpPeerState::DISCONNECTED || peer->state == UdpPeerState::ZOMBIE)
                continue;

            _header_flags = 0;
            _command_count = 0;
            _buffer_count = 1;
            _packet_size = sizeof(UdpProtocolHeader);

            //  ACKを返す
            // --------------------------------------------------

            if (!peer->acknowledgements.empty())
                _udp_protocol_send_acknowledgements(peer);

            //  ???
            // --------------------------------------------------

            if (check_for_timeouts &&
                !peer->sent_reliable_commands.empty() &&
                UDP_TIME_GREATER_EQUAL(_service_time, peer->next_timeout) &&
                _udp_protocol_check_timeouts(peer, event) == 1)
            {
                if (event->type != UdpEventType::NONE)
                    return 1;
                else
                    continue;
            }

            //  ???
            // --------------------------------------------------

            if ((!peer->outgoing_reliable_commands.empty() || _udp_protocol_send_reliable_outgoing_commands(peer)) &&
                peer->sent_reliable_commands.empty() &&
                UDP_TIME_DIFFERENCE(_service_time, peer->last_receive_time) >= peer->ping_interval &&
                peer->mtu - _packet_size >= sizeof(UdpProtocolPing))
            {
                udp_peer_ping(peer);
                _udp_protocol_send_reliable_outgoing_commands(peer);
            }

            //  ???
            // --------------------------------------------------

            if (!peer->outgoing_unreliable_commands.empty())
                _udp_protocol_send_unreliable_outgoing_commands(peer);

            if (_command_count == 0)
                continue;

            if (peer->packet_loss_epoch == 0)
            {
                peer->packet_loss_epoch = _service_time;
            }
            else if (UDP_TIME_DIFFERENCE(_service_time, peer->packet_loss_epoch) >= PEER_PACKET_LOSS_INTERVAL &&
                     peer->packets_sent > 0)
            {
                uint32_t packet_loss = peer->packets_lost * PEER_PACKET_LOSS_SCALE / peer->packets_sent;

                peer->packet_loss_variance -= peer->packet_loss_variance / 4;

                if (packet_loss >= peer->packet_loss)
                {
                    peer->packet_loss += (packet_loss - peer->packet_loss) / 8;
                    peer->packet_loss_variance += (packet_loss - peer->packet_loss) / 4;
                }
                else
                {
                    peer->packet_loss -= (peer->packet_loss - packet_loss) / 8;
                    peer->packet_loss_variance += (peer->packet_loss - packet_loss) / 4;
                }

                peer->packet_loss_epoch = _service_time;
                peer->packets_sent = 0;
                peer->packets_lost = 0;
            }

            if (_header_flags & PROTOCOL_HEADER_FLAG_SENT_TIME)
            {
                header->sent_time = htons(_service_time & 0xFFFF);
                _buffers[0].data_length = sizeof(UdpProtocolHeader);
            }
            else
            {
                _buffers[0].data_length = (size_t) & ((UdpProtocolHeader *) 0) -> sent_time; // ???
            }

            auto should_compress = false;

            if (_compressor->compress != nullptr)
            {
                 // ...
            }

            if (peer->outgoing_peer_id < PROTOCOL_MAXIMUM_PEER_ID)
                _header_flags |= peer->outgoing_session_id << PROTOCOL_HEADER_SESSION_SHIFT;

            header->peer_id = htons(peer->outgoing_peer_id | _header_flags);

            if (_checksum != nullptr)
            {
                // ...
            }

            if (should_compress)
            {
                // ...
            }

            peer->last_send_time = _service_time;

            auto sent_length = _udp_socket_send(peer->address);

            _udp_protocol_remove_sent_unreliable_commands(peer);

            if (sent_length < 0)
                return -1;

            _total_sent_data += sent_length;

            ++_total_sent_packets;
        }
    }

    return 0;
}

static void
init_event(std::unique_ptr<UdpEvent> &event)
{
    event->type = UdpEventType::NONE;
    event->peer = nullptr;
    event->packet = nullptr;
}

int
UdpHost::udp_host_service(std::unique_ptr<UdpEvent> &event, uint32_t timeout)
{
#define CHECK_RETURN_VALUE(val) \
    if (val == 1) \
        return 1; \
    else if (val == -1) \
        return -1;

    int ret;

    if (event != nullptr)
    {
        init_event(event);

        // - キューから取り出されたパケットは event に格納される
        // - ピアが取りうるステートは以下の 10 通りだが、この関数で処理されるのは 3,5,6,10 の４つ
        //  1. ACKNOWLEDGING_CONNECT,
        //  2. ACKNOWLEDGING_DISCONNECT,
        //  3. CONNECTED,
        //  4. CONNECTING,
        //  5. CONNECTION_PENDING,
        //  6. CONNECTION_SUCCEEDED,
        //  7. DISCONNECT_LATER,
        //  8. DISCONNECTED,
        //  9. DISCONNECTING,
        // 10. ZOMBIE
        ret = _udp_protocol_dispatch_incoming_commands(event);

        CHECK_RETURN_VALUE(ret)
    }

    _service_time = udp_time_get();

    timeout += _service_time;

    // 帯域幅の調整
    if (UDP_TIME_DIFFERENCE(_service_time, _bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        _udp_host_bandwidth_throttle();

    //
    ret = _protocol_send_outgoing_commands(event, true);

    CHECK_RETURN_VALUE(ret)

    //ret = protocol_receive_incoming_commands(host, event);

    CHECK_RETURN_VALUE(ret)

    //ret = protocol_send_outgoing_commands(host, event, 1);

    CHECK_RETURN_VALUE(ret)

    //ret = _udp_protocol_dispatch_incoming_commands(event);

    CHECK_RETURN_VALUE(ret)

    if (UDP_TIME_GREATER_EQUAL(_service_time, timeout))
        return 0;

    _service_time = udp_time_get();

    return 0;
}

UdpHost::UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth) :
    _checksum(nullptr),
    _bandwidth_limited_peers(0),
    _bandwidth_throttle_epoch(0),
    _buffer_count(0),
    _channel_count(channel_count),
    _command_count(0),
    _connected_peers(0),
    _duplicate_peers(PROTOCOL_MAXIMUM_PEER_ID),
    _incoming_bandwidth(in_bandwidth),
    _maximum_packet_size(HOST_DEFAULT_MAXIMUM_PACKET_SIZE),
    _maximum_waiting_data(HOST_DEFAULT_MAXIMUM_WAITING_DATA),
    _mtu(HOST_DEFAULT_MTU),
    _outgoing_bandwidth(out_bandwidth),
    _peer_count(peer_count),
    _peers(peer_count),
    _recalculate_bandwidth_limits(0),
    _received_address(std::make_unique<UdpAddress>()),
    _received_data_length(0),
    _total_received_data(0),
    _total_received_packets(0),
    _total_sent_data(0),
    _total_sent_packets(0),
    _packet_data(2, std::vector<uint8_t>(PROTOCOL_MAXIMUM_MTU))
{
    memset(_commands, 0, sizeof(_commands));

    _compressor = std::make_shared<UdpCompressor>();

    _socket = std::make_unique<Socket>();
    _socket->open(Socket::Type::UDP, IP::Type::ANY);
    _socket->set_blocking_enabled(false);

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
    {
        // throw exception
        // ...
    }

    if (_socket == nullptr || (_udp_socket_bind(_socket, address) != Error::OK))
    {
        // throw exception
        // ...
    }

    for (auto &peer : _peers)
    {
        peer->host = this;
        peer->incoming_peer_id = hash32();
        peer->outgoing_session_id = peer->incoming_session_id = 0xFF;
        peer->data = nullptr;

        udp_peer_reset(peer);
    }
}
