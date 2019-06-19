#include "RUdpPeerNet.h"
#include "RUdpPeerPod.h"

std::shared_ptr<UdpPeer>
UdpPeerPod::available_peer_exists()
{
    for (auto &peer : _peers)
    {
        if (peer->is_disconnected())
            return peer;
    }

    return nullptr;
}

UdpPeerPod::UdpPeerPod(size_t peer_count, std::shared_ptr<RUdpConnection> &conn) :
    _peers(peer_count),
    _peer_count(peer_count),
    _compressor(std::make_shared<UdpCompressor>()),
    _checksum(nullptr),
    _total_received_data(0),
    _total_received_packets(0),
    _total_sent_data(0),
    _total_sent_packets(0),
    _conn(conn)
{
    for (auto &peer : _peers)
        peer->reset();
}

int
UdpPeerPod::send_outgoing_commands(std::unique_ptr<UdpEvent> &event, uint32_t service_time, bool check_for_timeouts)
{
    uint8_t header_data[sizeof(RUdpProtocolHeader) + sizeof(uint32_t)];
    auto *header = reinterpret_cast<RUdpProtocolHeader *>(header_data);

    _protocol->continue_sending(true);

    while (_protocol->continue_sending())
    {
        _protocol->continue_sending(false);

        for (auto &peer : _peers)
        {
            if (peer->state_is(UdpPeerState::DISCONNECTED) || peer->state_is(UdpPeerState::ZOMBIE))
                continue;

            _protocol->chamber()->header_flags(0);
            _protocol->chamber()->command_count(0);
            _protocol->chamber()->buffer_count(1);
            _protocol->chamber()->packet_size(sizeof(RUdpProtocolHeader));

            //  ACKを返す
            // --------------------------------------------------

            if (peer->acknowledgement_exists())
                _protocol->send_acknowledgements(peer);

            //  タイムアウト処理
            // --------------------------------------------------

#define IS_EVENT_TYPE_NONE() \
    if (event->type != UdpEventType::NONE) \
        return 1; \
    else \
        continue;

            if (check_for_timeouts)
            {
                IS_EVENT_TYPE_NONE()
            }

            if (peer->command()->sent_reliable_command_exists())
            {
                IS_EVENT_TYPE_NONE()
            }

            if (UDP_TIME_GREATER_EQUAL(service_time, peer->command()->next_timeout()))
            {
                IS_EVENT_TYPE_NONE()
            }

            bool timed_out = peer->command()->check_timeouts(peer->net(), service_time);

            if (timed_out == 1)
            {
                _protocol->notify_disconnect(peer, event);

                IS_EVENT_TYPE_NONE()
            }

//            if (check_for_timeouts &&
//                !peer->sent_reliable_commands.empty() &&
//                UDP_TIME_GREATER_EQUAL(_service_time, peer->next_timeout) &&
//                _udp_protocol_check_timeouts(peer, event) == 1)
//            {
//                if (event->type != UdpEventType::NONE)
//                    return 1;
//                else
//                    continue;
//            }

            //  送信バッファに Reliable Command を転送する
            // --------------------------------------------------

            if ((peer->command()->outgoing_reliable_command_exists() || _protocol->_udp_protocol_send_reliable_outgoing_commands(peer, service_time)) &&
                !peer->command()->sent_reliable_command_exists() &&
                peer->exceeds_ping_interval(service_time) &&
                peer->exceeds_mtu(_protocol->chamber()->packet_size()))
            {
                peer->udp_peer_ping();

                // ping コマンドをバッファに転送
                _protocol->_udp_protocol_send_reliable_outgoing_commands(peer, service_time);
            }

            //  送信バッファに Unreliable Command を転送する
            // --------------------------------------------------

            if (peer->command()->outgoing_unreliable_command_exists())
                _protocol->_udp_protocol_send_unreliable_outgoing_commands(peer, service_time);

            //if (_command_count == 0)
            if (_protocol->chamber()->command_count() == 0)
                continue;

            if (peer->net()->packet_loss_epoch() == 0)
            {
                peer->net()->packet_loss_epoch(service_time);
            }
            else if (peer->net()->exceeds_packet_loss_interval(service_time) && peer->net()->packets_sent() > 0)
            {
                peer->net()->calculate_packet_loss(service_time);
            }

            if (_protocol->chamber()->header_flags() & PROTOCOL_HEADER_FLAG_SENT_TIME)
            {
                header->sent_time = htons(service_time & 0xFFFF);
                //_buffers[0].data_length = sizeof(RUdpProtocolHeader);
                _protocol->chamber()->set_data_length(sizeof(RUdpProtocolHeader));
            }
            else
            {
                //_buffers[0].data_length = (size_t) &((RUdpProtocolHeader *) 0)->sent_time; // ???
                _protocol->chamber()->set_data_length((size_t) &((RUdpProtocolHeader *) 0)->sent_time);
            }

            auto should_compress = false;

            if (_compressor->compress != nullptr)
            {
                // ...
            }

            if (peer->outgoing_peer_id() < PROTOCOL_MAXIMUM_PEER_ID)
            {
                auto header_flags = _protocol->chamber()->header_flags();
                header_flags |= peer->outgoing_session_id() << PROTOCOL_HEADER_SESSION_SHIFT;
                _protocol->chamber()->header_flags(header_flags);
            }

            header->peer_id = htons(peer->outgoing_peer_id() | _protocol->chamber()->header_flags());

            if (_checksum != nullptr)
            {
                // ...
            }

            if (should_compress)
            {
                // ...
            }

            peer->net()->last_send_time(service_time);

            auto sent_length = _conn->send(peer->address());

            peer->command()->remove_sent_unreliable_commands();

            if (sent_length < 0)
                return -1;

            _total_sent_data += sent_length;

            ++_total_sent_packets;
        }
    }

    return 0;
}

int
UdpPeerPod::protocol_dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event)
{
    return _protocol->dispatch_incoming_commands(event);
}

void
UdpPeerPod::protocol_bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth)
{
    _protocol->bandwidth_throttle(service_time, incoming_bandwidth, outgoing_bandwidth, _peers);
}

std::unique_ptr<RUdpProtocol> &
UdpPeerPod::protocol()
{
    return _protocol;
}
