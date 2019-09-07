#include "RUdpCommandSize.h"
#include "RUdpPeerNet.h"
#include "RUdpPeerPod.h"
#include "RUdpSegmentFlag.h"

RUdpPeerPod::RUdpPeerPod(size_t peer_count,
                         std::shared_ptr<RUdpConnection> &conn,
                         uint32_t host_incoming_bandwidth,
                         uint32_t host_outgoing_bandwidth)
    : checksum_(nullptr),
      compressor_(std::make_shared<RUdpCompressor>()),
      conn_(conn),
      duplicate_peers_(PROTOCOL_MAXIMUM_PEER_ID),
      host_incoming_bandwidth_(host_incoming_bandwidth),
      host_outgoing_bandwidth_(host_outgoing_bandwidth),
      intercept_(nullptr),
      peer_count_(peer_count),
      protocol_(std::make_unique<RUdpProtocol>()),
      received_address_(),
      received_data_(),
      received_data_length_(),
      segment_data_1_(PROTOCOL_MAXIMUM_MTU),
      segment_data_2_(PROTOCOL_MAXIMUM_MTU),
      service_time_(),
      total_received_data_(),
      total_received_segments_(),
      total_sent_data_(),
      total_sent_segments_()
{
    memset(&(segment_data_1_.at(0)), 0, sizeof(uint8_t) * PROTOCOL_MAXIMUM_MTU);
    memset(&(segment_data_2_.at(0)), 0, sizeof(uint8_t) * PROTOCOL_MAXIMUM_MTU);

    peers_.resize(peer_count);

    uint16_t idx = 0;
    for (auto &peer : peers_)
    {
        peer = std::make_unique<RUdpPeer>();
        peer->Reset(idx);
        idx++;
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

#define IS_EVENT_AVAILABLE() \
    if (event != nullptr && event->type != RUdpEventType::NONE) \
        return EventStatus::AN_EVENT_OCCURRED;                  \
    else                                                        \
        return EventStatus::NO_EVENT_OCCURRED;

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

        received_data_ = &segment_data_1_;
        received_data_length_ = received_length;

        total_received_data_ += received_length;
        total_received_segments_++;

        if (intercept_ == nullptr)
        {
            // ...
        }

        if (received_data_length_ < (size_t) &((RUdpProtocolHeader *) nullptr)->sent_time)
            continue;

        auto header = reinterpret_cast<RUdpProtocolHeader *>(&(received_data_->at(0)));
        auto peer_id = ntohs(header->peer_id);
        auto session_id = (peer_id & PROTOCOL_HEADER_SESSION_MASK) >> PROTOCOL_HEADER_SESSION_SHIFT;
        auto flags = peer_id & PROTOCOL_HEADER_FLAG_MASK;
        auto header_size =
            (flags & PROTOCOL_HEADER_FLAG_SENT_TIME ? sizeof(RUdpProtocolHeader) : (size_t) &((RUdpProtocolHeader *) 0)
                ->sent_time);

        peer_id &= ~(PROTOCOL_HEADER_FLAG_MASK | PROTOCOL_HEADER_SESSION_MASK);

        if (checksum_)
            header_size += sizeof(uint32_t);

        std::shared_ptr<RUdpPeer> peer;

        if (peer_id == PROTOCOL_MAXIMUM_PEER_ID)
        {
            peer = nullptr;
        }
        else if (peer_id >= peer_count_)
        {
            continue;
        }
        else
        {
            peer = peers_.at(peer_id); // TODO: How does this access the peers?

            if (!peer->EventOccur(received_address_, session_id))
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

        auto current_data = received_data_->begin() + header_size;

        while (current_data != received_data_->end())
        {
            if (current_data + sizeof(RUdpProtocolCommandHeader) > received_data_->end())
                break;

            auto cmd = reinterpret_cast<RUdpProtocolType *>(&(*current_data));

            auto cmd_number = cmd->header.command & PROTOCOL_COMMAND_MASK;
            if (cmd_number >= PROTOCOL_COMMAND_COUNT)
                break;

            auto cmd_size = command_sizes.at(cmd_number);
            if (cmd_size == 0 || current_data + cmd_size > received_data_->end())
                break;

            current_data += cmd_size;

            if (peer == nullptr && cmd_number != PROTOCOL_COMMAND_CONNECT)
                break;

            cmd->header.reliable_sequence_number = ntohs(cmd->header.reliable_sequence_number);

            if (cmd_number == PROTOCOL_COMMAND_ACKNOWLEDGE)
            {
                if (protocol_->HandleAcknowledge(event, peer, cmd) == Error::ERROR)
                {
                    IS_EVENT_AVAILABLE()
                }
            }
            else if (cmd_number == PROTOCOL_COMMAND_CONNECT)
            {
                if (peer)
                {
                    IS_EVENT_AVAILABLE()
                }

                auto duplicate_peers = 0;

                for (auto &p : peers_)
                {
                    if (p->StateIs(RUdpPeerState::DISCONNECTED))
                    {
                        peer = p;

                        break;
                    }
                    else if (p->StateIs(RUdpPeerState::CONNECTING) && p->address() == received_address_)
                    {
                        if (p->connect_id() == cmd->connect.connect_id)
                        {
                            peer = nullptr;

                            break;
                        }

                        ++duplicate_peers;
                    }
                }

                if (peer == nullptr || duplicate_peers >= duplicate_peers_)
                {
                    IS_EVENT_AVAILABLE()
                }

                protocol_->HandleConnect(peer,
                                         header,
                                         cmd,
                                         received_address_,
                                         host_incoming_bandwidth_,
                                         host_outgoing_bandwidth_);
            }
            else if (cmd_number == PROTOCOL_COMMAND_VERIFY_CONNECT)
            {
                if (protocol_->HandleVerifyConnect(event, peer, cmd) == Error::ERROR)
                {
                    IS_EVENT_AVAILABLE()
                }
            }
            else if (cmd_number == PROTOCOL_COMMAND_DISCONNECT)
            {
                if (protocol_->HandleDisconnect(peer, cmd) == Error::ERROR)
                {
                    IS_EVENT_AVAILABLE()
                }
            }
            else if (cmd_number == PROTOCOL_COMMAND_PING)
            {
//                if (protocol_->HandlePing(peer, cmd))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_RELIABLE)
            {
//                if (protocol_->HandleSendReliable(peer, cmd, current_data))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNRELIABLE)
            {
//                if (protocol_->HandleSendUnreliable(peer, cmd, current_data))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNSEQUENCED)
            {
//                if (protocol_->HandleSendUnsequenced(peer, cmd, current_data))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_FRAGMENT)
            {
//                if (protocol_->HandleSendFragment(peer, cmd, current_data))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_BANDWIDTH_LIMIT)
            {
//                if (protocol_->HandleBandwidthLimit(peer, cmd))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_THROTTLE_CONFIGURE)
            {
//                if (protocol_->HandleThrottleConfigure(peer, cmd))
//                    IS_EVENT_AVAILABLE()
            }
            else if (cmd_number == PROTOCOL_COMMAND_SEND_UNRELIABLE_FRAGMENT)
            {
//                if (protocol_->HandleSendUnreliableFragment(peer, cmd, current_data))
//                    IS_EVENT_AVAILABLE()
            }
            else
            {
                IS_EVENT_AVAILABLE()
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
                        peer->QueueAcknowledgement(cmd, sent_time);
                }
                else
                {
                    peer->QueueAcknowledgement(cmd, sent_time);
                }
            }
        }
    }

    return EventStatus::ERROR;
}

EventStatus
RUdpPeerPod::SendOutgoingCommands(const std::unique_ptr<RUdpEvent> &event,
                                  uint32_t service_time,
                                  bool check_for_timeouts)
{
    auto header_data = std::make_shared<std::vector<uint8_t>>(sizeof(RUdpProtocolHeader) + sizeof(uint32_t), 0);
    auto header = reinterpret_cast<RUdpProtocolHeader *>(&(header_data->at(0)));

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
            protocol_->chamber()->SetHeader(header_data);

            if (protocol_->chamber()->header_flags() & PROTOCOL_HEADER_FLAG_SENT_TIME)
            {
                header->sent_time = htons(service_time & 0xFFFF);
                //protocol_->chamber()->set_data_length(sizeof(RUdpProtocolHeader));
            }
            else
            {
                //protocol_->chamber()->set_data_length((size_t) &((RUdpProtocolHeader *) 0)->sent_time);
                protocol_->chamber()->DropSentTime();
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

            auto debug_outgoing_peer_id = peer->outgoing_peer_id();
            auto debug_header_flags = protocol_->chamber()->header_flags();

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

void
RUdpPeerPod::RequestPeerRemoval(size_t peer_idx, const std::shared_ptr<RUdpPeer> &peer)
{
    std::shared_ptr<RUdpSegment> segment = std::make_shared<RUdpSegment>(nullptr, static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE));

    segment->AddSysMsg(SysMsg::REMOVE_PEER);
    segment->AddPeerIdx(peer_idx);

    peer->Send(SysCh::CONFIG, segment, checksum_ != nullptr);
}

void
RUdpPeerPod::Flush()
{
    update_service_time();

    SendOutgoingCommands(nullptr, service_time_, true);
}
