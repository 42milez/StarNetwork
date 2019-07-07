#include "RUdpCommandSize.h"
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

#define COMMAND_ERROR() \
    if (event != nullptr && event->type != RUdpEventType::NONE) \
        return EventStatus::AN_EVENT_OCCURRED;

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
            //return EventStatus::NO_EVENT_OCCURRED;
            continue;

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
            //return EventStatus::NO_EVENT_OCCURRED;
            continue;
        }
        else
        {
            peer = peers_[peer_id];

            if (!peer->EventOccur(received_address_, session_id))
                //return EventStatus::NO_EVENT_OCCURRED;
                continue;
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
            peer->incoming_data_total(received_data_length_);
        }

        auto current_data = received_data_.begin() + header_size;

        while (current_data != received_data_.end())
        {
            auto cmd = reinterpret_cast<RUdpProtocolType *>(&(*current_data));

            if (current_data + sizeof(RUdpProtocolCommandHeader) > received_data_.end())
                break;

            auto cmd_number = cmd->header.command & PROTOCOL_COMMAND_MASK;
            if (cmd_number >= PROTOCOL_COMMAND_COUNT)
                break;

            auto cmd_size = command_sizes.at(cmd_number);
            if (cmd_size == 0 || current_data + cmd_size > received_data_.end())
                break;

            current_data += cmd_size;

            if (peer == nullptr && cmd_number != PROTOCOL_COMMAND_CONNECT)
                break;

            cmd->header.reliable_sequence_number = ntohs(cmd->header.reliable_sequence_number);

            if (cmd_number == PROTOCOL_COMMAND_ACKNOWLEDGE)
            {
                if (protocol_->HandleAcknowledge(event, peer, cmd))
                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_CONNECT)
            {
//                if (peer)
//                    COMMAND_ERROR()
//
//                peer = protocol_->HandleConnect(header, cmd);
//
//                if (peer == nullptr)
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_VERIFY_CONNECT)
            {
//                if (protocol_->HandleVerifyConnect(event, peer, cmd))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_DISCONNECT)
            {
//                if (protocol_->HandleDisconnect(peer, cmd))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_PING)
            {
//                if (protocol_->HandlePing(peer, cmd))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_RELIABLE)
            {
//                if (protocol_->HandleSendReliable(peer, cmd, current_data))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNRELIABLE)
            {
//                if (protocol_->HandleSendUnreliable(peer, cmd, current_data))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNSEQUENCED)
            {
//                if (protocol_->HandleSendUnsequenced(peer, cmd, current_data))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_FRAGMENT)
            {
//                if (protocol_->HandleSendFragment(peer, cmd, current_data))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_BANDWIDTH_LIMIT)
            {
//                if (protocol_->HandleBandwidthLimit(peer, cmd))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_THROTTLE_CONFIGURE)
            {
//                if (protocol_->HandleThrottleConfigure(peer, cmd))
//                    COMMAND_ERROR()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNRELIABLE_FRAGMENT)
            {
//                if (protocol_->HandleSendUnreliableFragment(peer, cmd, current_data))
//                    COMMAND_ERROR()
            }
            else
            {
                COMMAND_ERROR()
            }

            if (peer && (cmd->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE) != 0)
            {
                if (!(flags & PROTOCOL_HEADER_FLAG_SENT_TIME))
                    break;

                auto sent_time = ntohs(header->sent_time);

                if (peer->StateIs(RUdpPeerState::DISCONNECTING) ||
                    peer->StateIs(RUdpPeerState::ACKNOWLEDGING_CONNECT) ||
                    peer->StateIs(RUdpPeerState::DISCONNECTED) ||
                    peer->StateIs(RUdpPeerState::ZOMBIE))
                {
                    // DO NOTHING
                }
                else if (peer->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT))
                {
                    if ((cmd->header.command & PROTOCOL_COMMAND_MASK) == PROTOCOL_COMMAND_DISCONNECT)
                        peer->QueueAcknowledgement(peer, cmd, sent_time);
                }
                else
                {
                    peer->QueueAcknowledgement(peer, cmd, sent_time);
                }
            }
        }
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
