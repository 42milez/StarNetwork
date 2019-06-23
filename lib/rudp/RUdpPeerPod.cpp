#include "RUdpPeerNet.h"
#include "RUdpPeerPod.h"

RUdpPeerPod::RUdpPeerPod(size_t peer_count, std::shared_ptr<RUdpConnection> &conn)
    :
    checksum_(nullptr),
    compressor_(std::make_shared<RUdpCompressor>()),
    conn_(conn),
    peer_count_(peer_count),
    total_received_data_(),
    total_received_segments_(),
    total_sent_data_(),
    total_sent_segments_()
{
    peers_.resize(peer_count);

    for (auto &peer : peers_)
    {
        peer = std::make_unique<RUdpPeer>();
        peer->reset();
    }
}

std::shared_ptr<RUdpPeer>
RUdpPeerPod::AvailablePeerExists()
{
    for (auto &peer : peers_) {
        if (peer->is_disconnected())
            return peer;
    }

    return nullptr;
}

void
RUdpPeerPod::BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth,
                               uint32_t outgoing_bandwidth)
{
    protocol_->bandwidth_throttle(service_time, incoming_bandwidth, outgoing_bandwidth, peers_);
}

int
RUdpPeerPod::DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event)
{
    return protocol_->dispatch_incoming_commands(event);
}

#define IS_EVENT_TYPE_NONE() \
    if (event->type != RUdpEventType::NONE) \
        return 1; \
    else \
        continue;

int
RUdpPeerPod::SendOutgoingCommands(std::unique_ptr<RUdpEvent> &event, uint32_t service_time, bool check_for_timeouts)
{
    uint8_t header_data[sizeof(RUdpProtocolHeader) + sizeof(uint32_t)];
    auto *header = reinterpret_cast<RUdpProtocolHeader *>(header_data);

    protocol_->continue_sending(true);

    while (protocol_->continue_sending()) {
        protocol_->continue_sending(false);

        for (auto &peer : peers_) {
            if (peer->state_is(RUdpPeerState::DISCONNECTED) || peer->state_is(RUdpPeerState::ZOMBIE))
                continue;

            protocol_->chamber()->header_flags(0);
            protocol_->chamber()->command_count(0);
            protocol_->chamber()->buffer_count(1);
            protocol_->chamber()->segment_size(sizeof(RUdpProtocolHeader));

            //  ACKを返す
            // --------------------------------------------------

            if (peer->acknowledgement_exists())
                protocol_->send_acknowledgements(peer);

            //  タイムアウト処理
            // --------------------------------------------------

            if (check_for_timeouts) {
                IS_EVENT_TYPE_NONE()
            }

            if (peer->command()->sent_reliable_command_exists()) {
                IS_EVENT_TYPE_NONE()
            }

            if (UDP_TIME_GREATER_EQUAL(service_time, peer->command()->next_timeout())) {
                IS_EVENT_TYPE_NONE()
            }

            bool timed_out = peer->command()->check_timeouts(peer->net(), service_time);

            if (timed_out == 1) {
                protocol_->notify_disconnect(peer, event);

                IS_EVENT_TYPE_NONE()
            }

//            if (check_for_timeouts &&
//                !peer->sent_reliable_commands.empty() &&
//                UDP_TIME_GREATER_EQUAL(_service_time, peer->next_timeout) &&
//                _udpprotocol__check_timeouts(peer, event) == 1)
//            {
//                if (event->type != RUdpEventType::NONE)
//                    return 1;
//                else
//                    continue;
//            }

            //  送信バッファに Reliable Command を転送する
            // --------------------------------------------------

            if ((peer->command()->outgoing_reliable_command_exists() ||
                protocol_->send_reliable_outgoing_commands(peer, service_time)) &&
                !peer->command()->sent_reliable_command_exists() &&
                peer->exceeds_ping_interval(service_time) &&
                peer->exceeds_mtu(protocol_->chamber()->segment_size())) {
                peer->udp_peer_ping();

                // ping コマンドをバッファに転送
                protocol_->send_reliable_outgoing_commands(peer, service_time);
            }

            //  送信バッファに Unreliable Command を転送する
            // --------------------------------------------------

            if (peer->command()->outgoing_unreliable_command_exists())
                protocol_->send_unreliable_outgoing_commands(peer, service_time);

            //if (_command_count == 0)
            if (protocol_->chamber()->command_count() == 0)
                continue;

            if (peer->net()->segment_loss_epoch() == 0) {
                peer->net()->segment_loss_epoch(service_time);
            }
            else if (peer->net()->exceeds_segment_loss_interval(service_time) && peer->net()->segments_sent() > 0) {
                peer->net()->calculate_segment_loss(service_time);
            }

            if (protocol_->chamber()->header_flags() & PROTOCOL_HEADER_FLAG_SENT_TIME) {
                header->sent_time = htons(service_time & 0xFFFF);
                //_buffers[0].data_length = sizeof(RUdpProtocolHeader);
                protocol_->chamber()->set_data_length(sizeof(RUdpProtocolHeader));
            }
            else {
                //_buffers[0].data_length = (size_t) &((RUdpProtocolHeader *) 0)->sent_time; // ???
                protocol_->chamber()->set_data_length((size_t) &((RUdpProtocolHeader *) 0)->sent_time);
            }

            auto should_compress = false;

            if (compressor_->compress != nullptr) {
                // ...
            }

            if (peer->outgoing_peer_id() < PROTOCOL_MAXIMUM_PEER_ID) {
                auto header_flags = protocol_->chamber()->header_flags();
                header_flags |= peer->outgoing_session_id() << PROTOCOL_HEADER_SESSION_SHIFT;
                protocol_->chamber()->header_flags(header_flags);
            }

            header->peer_id = htons(peer->outgoing_peer_id() | protocol_->chamber()->header_flags());

            if (checksum_ != nullptr) {
                // ...
            }

            if (should_compress) {
                // ...
            }

            peer->net()->last_send_time(service_time);

            auto sent_length = conn_->send(peer->address(), protocol_->chamber());

            peer->command()->remove_sent_unreliable_commands();

            if (sent_length < 0)
                return -1;

            total_sent_data_ += sent_length;

            ++total_sent_segments_;
        }
    }

    return 0;
}
