#include "RUdpPeerNet.h"
#include "RUdpPeerPod.h"

RUdpPeerPod::RUdpPeerPod(size_t peer_count, std::shared_ptr<RUdpConnection> &conn)
    :
    checksum_(nullptr),
    compressor_(std::make_shared<RUdpCompressor>()),
    conn_(conn),
    intercept_(nullptr),
    peer_count_(peer_count),
    protocol_(std::make_unique<RUdpProtocol>()),
    received_address_(),
    received_data_(),
    received_data_length_(),
    segment_data_1_(),
    segment_data_2_(),
    total_received_data_(),
    total_received_segments_(),
    total_sent_data_(),
    total_sent_segments_()
{
    peers_.resize(peer_count);

    for (auto &peer : peers_)
    {
        peer = std::make_unique<RUdpPeer>();
        peer->Reset();
    }
}

std::shared_ptr<RUdpPeer>
RUdpPeerPod::AvailablePeer()
{
    for (auto &peer : peers_)
    {
        if (peer->Disconnected())
            return peer;
    }

    return nullptr;
}

void
RUdpPeerPod::BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth,
                               uint32_t outgoing_bandwidth)
{
    protocol_->BandwidthThrottle(service_time, incoming_bandwidth, outgoing_bandwidth, peers_);
}

EventStatus
RUdpPeerPod::DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event)
{
    return protocol_->DispatchIncomingCommands(event);
}

#define IS_EVENT_TYPE_NONE() \
    if (event->type != RUdpEventType::NONE) \
        return EventStatus::AN_EVENT_OCCURRED; \
    else \
        continue;

EventStatus
RUdpPeerPod::ReceiveIncomingCommands(std::unique_ptr<RUdpEvent> &event)
{
    for (auto i = 0; i < 256; ++i)
    {
        auto received_length = conn_->receive(received_address_, segment_data_1_, 1);

        if (received_length < 0)
            return EventStatus::ERROR;

        if (received_length == 0)
            return EventStatus::NO_EVENT_OCCURRED;

        total_received_data_ += segment_data_1_.size();

        ++total_received_segments_;

        if (intercept_)
        {
            // ...
        }

        // ---------- [ enet_protocol_handle_incoming_commands ] ----------

        /*

           switch (enet_protocol_handle_incoming_commands (host, event))
           {
           case 1:
              return 1;

           case -1:
              return -1;

           default:
              break;
           }

         */

        if (received_data_length_ < (size_t) & ((RUdpProtocolHeader *) 0) -> sent_time)
            return EventStatus::NO_EVENT_OCCURRED;

        auto header = reinterpret_cast<RUdpProtocolHeader *>(&received_data_);
        auto peer_id = header->peer_id;
        auto session_id = (peer_id & PROTOCOL_HEADER_SESSION_MASK) >> PROTOCOL_HEADER_SESSION_SHIFT;
        auto flags = peer_id & PROTOCOL_HEADER_FLAG_MASK;
        auto header_size = (flags & PROTOCOL_HEADER_FLAG_SENT_TIME ? sizeof(RUdpProtocolHeader) : (size_t) & ((RUdpProtocolHeader *) 0) -> sent_time);

        peer_id &= (PROTOCOL_HEADER_FLAG_MASK | PROTOCOL_HEADER_SESSION_MASK);

        if (checksum_)
            header_size += sizeof(uint32_t);

        std::shared_ptr<RUdpPeer> peer;

        if (peer_id == PROTOCOL_MAXIMUM_PEER_ID)
        {
            peer = nullptr;
        }
        else if (peer_id >= peer_count_)
        {
            return EventStatus::NO_EVENT_OCCURRED;
        }
        else
        {
            peer = peers_[peer_id];

            if (!peer->EventOccur(received_address_, session_id))
                return EventStatus::NO_EVENT_OCCURRED;
        }

        if (flags & PROTOCOL_HEADER_FLAG_COMPRESSED)
        {
            // ...
        }

        if (checksum_)
        {
            // ...
        }

        if (peer)
        {
            peer->address(received_address_);
        }

        // ...
    }

    return EventStatus::ERROR;
}

EventStatus
RUdpPeerPod::SendOutgoingCommands(std::unique_ptr<RUdpEvent> &event, uint32_t service_time, bool check_for_timeouts)
{
    auto header_data_size = sizeof(RUdpProtocolHeader) + sizeof(uint32_t);
    uint8_t header_data[header_data_size];
    auto *header = reinterpret_cast<RUdpProtocolHeader *>(header_data);

    protocol_->continue_sending(true);

    while (protocol_->continue_sending())
    {
        protocol_->continue_sending(false);

        for (auto &peer : peers_)
        {
            if (peer->StateIs(RUdpPeerState::DISCONNECTED) || peer->StateIs(RUdpPeerState::ZOMBIE))
                continue;

            protocol_->chamber()->header_flags(0);
            protocol_->chamber()->command_count(0);
            protocol_->chamber()->buffer_count(1);
            protocol_->chamber()->segment_size(sizeof(RUdpProtocolHeader));

            //  ACKを返す
            // --------------------------------------------------

            if (peer->AcknowledgementExists())
                protocol_->SendAcknowledgements(peer);

            //  タイムアウト処理
            // --------------------------------------------------

            if (check_for_timeouts &&
                peer->command()->sent_reliable_command_exists() &&
                UDP_TIME_GREATER_EQUAL(service_time, peer->command()->next_timeout()) &&
                peer->command()->check_timeouts(peer->net(), service_time) == 1)
            {
                IS_EVENT_TYPE_NONE()
            }

            //  送信バッファに Reliable Command を転送する
            // --------------------------------------------------

            if ((!peer->command()->outgoing_reliable_command_exists() ||
                protocol_->SendReliableOutgoingCommands(peer, service_time)) &&
                !peer->command()->sent_reliable_command_exists() &&
                peer->ExceedsPingInterval(service_time) &&
                peer->ExceedsMTU(protocol_->chamber()->segment_size()))
            {
                peer->Ping();

                // ping コマンドをバッファに転送
                protocol_->SendReliableOutgoingCommands(peer, service_time);
            }

            //  送信バッファに Unreliable Command を転送する
            // --------------------------------------------------

            if (peer->command()->outgoing_unreliable_command_exists())
                protocol_->SendUnreliableOutgoingCommands(peer, service_time);

            if (protocol_->chamber()->command_count() == 0)
                continue;

            if (peer->net()->segment_loss_epoch() == 0)
            {
                peer->net()->segment_loss_epoch(service_time);
            }
            else if (peer->net()->exceeds_segment_loss_interval(service_time) && peer->net()->segments_sent() > 0)
            {
                peer->net()->calculate_segment_loss(service_time);
            }

            // ⚠️ _buffers[0]には必ずヘッダが設定される。なので、_buffersは以下の構造となる
            //
            // _buffers[0]: ヘッダ
            // _buffers[1]: コマンド
            // _buffers[2]: コマンド
            // _buffers[n]: コマンド
            protocol_->chamber()->copy_header_data(header_data, header_data_size);

            if (protocol_->chamber()->header_flags() & PROTOCOL_HEADER_FLAG_SENT_TIME)
            {
                header->sent_time = htons(service_time & 0xFFFF);
                //protocol_->chamber()->set_data_length(sizeof(RUdpProtocolHeader));
            }
            else
            {
                //protocol_->chamber()->set_data_length((size_t) &((RUdpProtocolHeader *) 0)->sent_time);
            }

            auto should_compress = false;

            if (compressor_->compress != nullptr)
            {
                // ...
            }

            if (peer->outgoing_peer_id() < PROTOCOL_MAXIMUM_PEER_ID)
            {
                auto header_flags = protocol_->chamber()->header_flags();
                header_flags |= peer->outgoing_session_id() << PROTOCOL_HEADER_SESSION_SHIFT;
                protocol_->chamber()->header_flags(header_flags);
            }

            header->peer_id = htons(peer->outgoing_peer_id() | protocol_->chamber()->header_flags());

            if (checksum_ != nullptr)
            {
                // ...
            }

            if (should_compress)
            {
                // ...
            }

            peer->net()->last_send_time(service_time);

            auto sent_length = conn_->send(peer->address(), protocol_->chamber());

            peer->command()->remove_sent_unreliable_commands();

            if (sent_length < 0)
                return EventStatus::ERROR;

            total_sent_data_ += sent_length;

            ++total_sent_segments_;
        }
    }

    return EventStatus::NO_EVENT_OCCURRED;
}
