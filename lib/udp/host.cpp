#include "core/hash.h"

#include "host.h"
#include "peer.h"
#include "udp.h"

void
udp_host_compress(std::shared_ptr<UdpHost> &host)
{
    if (host->compressor->destroy != nullptr)
        host->compressor->destroy();
}

void
udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor)
{
    if (compressor)
        host->compressor = compressor;
}

std::shared_ptr<UdpHost>
udp_host_create(const std::unique_ptr<UdpAddress> &address, size_t peer_count, SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    std::shared_ptr<UdpHost> host;

    if (peer_count > PROTOCOL_MAXIMUM_PEER_ID)
        return nullptr;

    host = std::make_shared<UdpHost>(channel_count, in_bandwidth, out_bandwidth, peer_count);

    if (host == nullptr)
        return nullptr;

    if (host->socket == nullptr || (address != nullptr && udp_socket_bind(host->socket, address) != Error::OK))
        return nullptr;

    for (auto &peer : host->peers)
    {
        peer.host = host;
        peer.incoming_peer_id = hash32();
        peer.outgoing_session_id = peer.incoming_session_id = 0xFF;
        peer.data = nullptr;

        udp_peer_reset(peer);
    }

    return host;
}

Error
udp_host_connect(std::shared_ptr<UdpHost> &host, const UdpAddress &address, SysCh channel_count, uint32_t data)
{
    auto current_peer = host->peers.begin();

    for (; current_peer != host->peers.end(); ++current_peer)
    {
        if (current_peer->state == UdpPeerState::DISCONNECTED)
            break;
    }

    if (current_peer == host->peers.end())
        return Error::CANT_CREATE;

    current_peer->channels = std::move(std::vector<UdpChannel>(static_cast<int>(channel_count)));

    if (current_peer->channels.empty())
        return Error::CANT_CREATE;

    current_peer->state = UdpPeerState::CONNECTING;
    current_peer->address = address;
    current_peer->connect_id = hash32();

    if (host->outgoing_bandwidth == 0)
    {
        current_peer->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    }
    else
    {
        current_peer->window_size = (host->outgoing_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;
    }

    if (current_peer->window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        current_peer->window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (current_peer->window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        current_peer->window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    std::shared_ptr<UdpProtocol> cmd;

    cmd->header.command = PROTOCOL_COMMAND_CONNECT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    cmd->header.channel_id = 0xFF;

    cmd->connect.outgoing_peer_id = htons(current_peer->incoming_peer_id);
    cmd->connect.incoming_session_id = current_peer->incoming_session_id;
    cmd->connect.outgoing_session_id = current_peer->outgoing_session_id;
    cmd->connect.mtu = htonl(current_peer->mtu);
    cmd->connect.window_size = htonl(current_peer->window_size);
    cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
    cmd->connect.incoming_bandwidth = htonl(host->incoming_bandwidth);
    cmd->connect.outgoing_bandwidth = htonl(host->outgoing_bandwidth);
    cmd->connect.packet_throttle_interval = htonl(current_peer->packet_throttle_interval);
    cmd->connect.packet_throttle_acceleration = htonl(current_peer->packet_throttle_acceleration);
    cmd->connect.packet_throttle_deceleration = htonl(current_peer->packet_throttle_deceleration);
    cmd->connect.data = data;

    udp_peer_queue_outgoing_command(*current_peer, cmd, nullptr, 0, 0);

    return Error::OK;
}

void
udp_host_bandwidth_throttle(std::shared_ptr<UdpHost> &host)
{
    auto time_current = udp_time_get();
    auto time_elapsed = time_current - host->bandwidth_throttle_epoch;
    auto peers_remaining = host->connected_peers;
    auto data_total = ~0u;
    auto bandwidth = ~0u;
    auto throttle = 0;
    auto bandwidth_limit = 0;
    auto needs_adjustment = host->bandwidth_limited_peers > 0 ? true : false;

    if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
        return;

    host->bandwidth_throttle_epoch = time_current;

    if (peers_remaining == 0)
        return;

    //  Throttle outgoing bandwidth
    // --------------------------------------------------

    if (host->outgoing_bandwidth != 0)
    {
        data_total = 0;
        bandwidth = host->outgoing_bandwidth * (time_elapsed / 1000);

        for (const auto &peer : host->peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            data_total += peer.outgoing_data_total;
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

        for (auto &peer : host->peers)
        {
            uint32_t peer_bandwidth;

            if ((IS_PEER_CONNECTED(peer)) ||
                peer.incoming_bandwidth == 0 ||
                peer.outgoing_bandwidth_throttle_epoch == time_current)
            {
                continue;
            }

            peer_bandwidth = peer.incoming_bandwidth * (time_elapsed / 1000);
            if ((throttle * peer.outgoing_data_total) / PEER_PACKET_THROTTLE_SCALE <= peer_bandwidth)
                continue;

            peer.packet_throttle_limit = (peer_bandwidth * PEER_PACKET_THROTTLE_SCALE) / peer.outgoing_data_total;

            if (peer.packet_throttle_limit == 0)
                peer.packet_throttle_limit = 1;

            if (peer.packet_throttle > peer.packet_throttle_limit)
                peer.packet_throttle = peer.packet_throttle_limit;

            peer.outgoing_bandwidth_throttle_epoch = time_current;
            peer.incoming_data_total = 0;
            peer.outgoing_data_total = 0;

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

        for (auto &peer : host->peers)
        {
            if ((IS_PEER_CONNECTED(peer)) ||
                peer.outgoing_bandwidth_throttle_epoch == time_current)
            {
                continue;
            }

            peer.packet_throttle_limit = throttle;

            if (peer.packet_throttle > peer.packet_throttle_limit)
                peer.packet_throttle = peer.packet_throttle_limit;

            peer.incoming_data_total = 0;
            peer.outgoing_data_total = 0;
        }
    }

    //  Recalculate Bandwidth Limits
    // --------------------------------------------------

    if (host->recalculate_bandwidth_limits)
    {
        host->recalculate_bandwidth_limits = false;
        peers_remaining = host->connected_peers;
        bandwidth = host->incoming_bandwidth;
        needs_adjustment = true;

        if (bandwidth == 0)
            bandwidth_limit = 0;
        else
            while (peers_remaining > 0 && needs_adjustment)
            {
                needs_adjustment = false;
                bandwidth_limit = bandwidth / peers_remaining;

                for (auto &peer: host->peers)
                {
                    if ((IS_PEER_CONNECTED(peer)) ||
                        peer.incoming_bandwidth_throttle_epoch == time_current)
                    {
                        continue;
                    }

                    if (peer.outgoing_bandwidth > 0 && peer.outgoing_bandwidth >= bandwidth_limit)
                        continue;

                    peer.incoming_bandwidth_throttle_epoch = time_current;

                    needs_adjustment = true;

                    --peers_remaining;

                    bandwidth -= peer.outgoing_bandwidth;
                }
            }


        std::shared_ptr<UdpProtocol> cmd;

        for (auto &peer : host->peers)
        {
            if (IS_PEER_CONNECTED(peer))
                continue;

            cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
            cmd->header.channel_id = 0xFF;
            cmd->bandwidth_limit.outgoing_bandwidth = htonl(host->outgoing_bandwidth);

            if (peer.incoming_bandwidth_throttle_epoch == time_current)
                cmd->bandwidth_limit.incoming_bandwidth = htonl(peer.outgoing_bandwidth);
            else
                cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

            udp_peer_queue_outgoing_command(peer, cmd, nullptr, 0, 0);
        }
    }
}

int
udp_host_service(std::shared_ptr<UdpHost> &host, UdpEvent &event, uint32_t timeout)
{
#define CHECK_RETURN_VALUE(val) \
    if (val == 1) \
        return 1; \
    else if (val == -1) \
        return -1;

    int ret;

    if (event.type == UdpEventType::NONE)
    {
        ret = udp_protocol_dispatch_incoming_commands(host, event);

        CHECK_RETURN_VALUE(ret)
    }

    host->service_time = udp_time_get();

    timeout += host->service_time;

    if (UDP_TIME_DIFFERENCE(host->service_time, host->bandwidth_throttle_epoch) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        udp_host_bandwidth_throttle(host);

    //ret = udp_protocol_send_outgoing_commands(host, event, 1);

    CHECK_RETURN_VALUE(ret)

    //ret = udp_protocol_receive_incoming_commands(host, event);

    CHECK_RETURN_VALUE(ret)

    //ret = udp_protocol_send_outgoing_commands(host, event, 1);

    CHECK_RETURN_VALUE(ret)

    ret = udp_protocol_dispatch_incoming_commands(host, event);

    CHECK_RETURN_VALUE(ret)

    if (UDP_TIME_GREATER_EQUAL(host->service_time, timeout))
        return 0;

    host->service_time = udp_time_get();

    return 0;
}
