#ifdef __linux__
#include <arpa/inet.h>
#endif

#include "lib/core/network/system.h"
#include "lib/rudp/command/command_size.h"
#include "lib/rudp/enum.h"
#include "peer_net.h"
#include "peer_pod.h"

namespace rudp
{
    PeerPod::PeerPod(size_t peer_count, std::shared_ptr<Connection> &conn, uint32_t host_incoming_bandwidth,
                     uint32_t host_outgoing_bandwidth)
        : compressor_(std::make_shared<Compress>())
        , conn_(conn)
        , duplicate_peers_(PROTOCOL_MAXIMUM_PEER_ID)
        , host_incoming_bandwidth_(host_incoming_bandwidth)
        , host_outgoing_bandwidth_(host_outgoing_bandwidth)
        , intercept_()
        , peer_count_(peer_count)
        , prev_service_time_()
        , protocol_(std::make_unique<RUdpProtocol>())
        , received_address_()
        , received_data_()
        , received_data_length_()
        , segment_data_1_(PROTOCOL_MAXIMUM_MTU)
        , segment_data_2_(PROTOCOL_MAXIMUM_MTU)
        , service_time_()
        , total_received_data_()
        , total_received_segments_()
        , total_sent_data_()
        , total_sent_segments_()
    {
        peers_.resize(peer_count);
        segment_data_1_.resize(PROTOCOL_MAXIMUM_MTU);
        segment_data_2_.resize(PROTOCOL_MAXIMUM_MTU);

        uint16_t idx = 0;
        for (auto &peer : peers_) {
            peer = std::make_unique<Peer>();
            peer->Reset(idx);
            idx++;
        }
    }

    std::shared_ptr<Peer>
    PeerPod::AvailablePeer()
    {
        for (auto &peer : peers_) {
            if (peer->Disconnected())
                return peer;
        }

        return nullptr;
    }

#define IS_EVENT_TYPE_NONE()                                                                                           \
    if (event->TypeIsNot(EventType::NONE))                                                                             \
        return EventStatus::AN_EVENT_OCCURRED;                                                                         \
    else                                                                                                               \
        continue;

#define IS_EVENT_AVAILABLE()                                                                                           \
    if (event != nullptr && event->TypeIsNot(EventType::NONE))                                                         \
        return EventStatus::AN_EVENT_OCCURRED;                                                                         \
    else                                                                                                               \
        return EventStatus::NO_EVENT_OCCURRED;

    Error
    PeerPod::Disconnect(const std::shared_ptr<Peer> &peer, uint32_t data, ChecksumCallback checksum)
    {
        auto &net = peer->net();

        if (net->StateIs(RUdpPeerState::DISCONNECTING) || net->StateIs(RUdpPeerState::DISCONNECTED) ||
            net->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT) || net->StateIs(RUdpPeerState::ZOMBIE)) {
            return Error::ERROR;
        }

        peer->ResetPeerQueues();

        auto cmd               = std::make_shared<ProtocolType>();
        cmd->header.command    = static_cast<uint8_t>(RUdpProtocolCommand::DISCONNECT);
        cmd->header.channel_id = 0xFF;
        cmd->disconnect.data   = htonl(data);

        if (net->StateIs(RUdpPeerState::CONNECTED) || net->StateIs(RUdpPeerState::DISCONNECT_LATER))
            cmd->header.command |= static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
        else
            cmd->header.command |= static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_UNSEQUENCED);

        peer->QueueOutgoingCommand(cmd, nullptr, 0);

        if (net->StateIs(RUdpPeerState::CONNECTED) || net->StateIs(RUdpPeerState::DISCONNECT_LATER)) {
            PeerOnDisconnect(peer);
            net->state(RUdpPeerState::DISCONNECTING);
        }
        else {
            Flush(checksum);
            peer->Reset();
        }

        return Error::OK;
    }

    Error
    PeerPod::DisconnectLater(const std::shared_ptr<Peer> &peer, uint32_t data, ChecksumCallback checksum)
    {
        auto &net     = peer->net();
        auto &cmd_pod = peer->command_pod();

        if ((net->StateIs(RUdpPeerState::CONNECTED) || net->StateIs(RUdpPeerState::DISCONNECT_LATER)) &&
            (cmd_pod->OutgoingReliableCommandExists() || cmd_pod->OutgoingUnreliableCommandExists() ||
             cmd_pod->SentReliableCommandExists())) {
            net->state(RUdpPeerState::DISCONNECT_LATER);
            peer->event_data(data);
        }
        else {
            Disconnect(peer, data, checksum);
        }

        return Error::OK;
    }

    Error
    PeerPod::DisconnectNow(const std::shared_ptr<Peer> &peer, uint32_t data, ChecksumCallback checksum)
    {
        auto &net = peer->net();

        if (net->StateIs(RUdpPeerState::DISCONNECTED))
            return Error::ERROR;

        std::shared_ptr<ProtocolType> cmd = std::make_shared<ProtocolType>();

        if (net->StateIsNot(RUdpPeerState::ZOMBIE) && net->StateIsNot(RUdpPeerState::DISCONNECTING)) {
            peer->ResetPeerQueues();

            cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::DISCONNECT) |
                                  static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_UNSEQUENCED);
            cmd->header.channel_id = 0xFF;
            cmd->disconnect.data   = htonl(data);

            peer->QueueOutgoingCommand(cmd, nullptr, 0);

            Flush(checksum);
        }

        peer->Reset();

        return Error::OK;
    }

    void
    PeerPod::Flush(ChecksumCallback checksum)
    {
        UpdateServiceTime();

        SendOutgoingCommands(nullptr, service_time_, false, checksum);
    }

    EventStatus
    PeerPod::ReceiveIncomingCommands(std::unique_ptr<Event> &event, ChecksumCallback checksum)
    {
        for (auto i = 0; i < 256; ++i) {
            auto received_length = conn_->Receive(received_address_, segment_data_1_, 1);

            if (received_length < 0)
                return EventStatus::ERROR;

            if (received_length == 0)
                return EventStatus::NO_EVENT_OCCURRED;

            received_data_        = &segment_data_1_;
            received_data_length_ = received_length;

            total_received_data_ += received_length;
            total_received_segments_++;

            if (intercept_ == nullptr) {
                // ...
            }

            if (received_data_length_ < (size_t) & ((ProtocolHeader *)nullptr)->sent_time)
                continue;

            auto header =
                ConvertNetworkByteOrderToHostByteOrder(reinterpret_cast<ProtocolHeader *>(&(received_data_->at(0))));
            auto peer_id    = header->peer_id;
            auto session_id = (peer_id & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK)) >>
                              static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT);
            auto flags       = peer_id & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_MASK);
            auto header_size = (flags & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME)
                                    ? sizeof(ProtocolHeader)
                                    : (size_t) & ((ProtocolHeader *)0)->sent_time);

            peer_id &= ~(static_cast<uint16_t>(RUdpProtocolFlag::HEADER_MASK) |
                         static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK));

            if (checksum)
                header_size += sizeof(uint32_t);

            std::shared_ptr<Peer> peer;

            if (peer_id == PROTOCOL_MAXIMUM_PEER_ID) {
                peer = nullptr;
            }
            else if (peer_id >= peer_count_) {
                continue;
            }
            else {
                peer = peers_.at(peer_id);

                if (!peer->EventOccur(received_address_, session_id))
                    continue;
            }

            if (flags & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_COMPRESSED)) {
                // ...
            }

            if (checksum) {
                // ...
            }

            if (peer) {
                peer->Address(received_address_);
                peer->command_pod()->IncrementIncomingDataTotal(received_data_length_);
            }

            auto current_data = received_data_->begin() + header_size;
            auto end          = received_data_->begin() + received_data_length_;

            while (current_data < end) {
                if (current_data + sizeof(ProtocolCommandHeader) > end)
                    break;

                auto cmd_raw =
                    ConvertNetworkByteOrderToHostByteOrder(reinterpret_cast<ProtocolType *>(&(*current_data)));
                auto cmd      = std::make_shared<ProtocolType>(*cmd_raw);
                auto cmd_type = static_cast<RUdpProtocolCommand>(cmd->header.command & PROTOCOL_COMMAND_MASK);

                if (cmd_type >= RUdpProtocolCommand::COUNT)
                    break;

                auto cmd_size = COMMAND_SIZES.at(static_cast<uint8_t>(cmd_type));
                if (cmd_size == 0 || current_data + cmd_size > end)
                    break;

                current_data += cmd_size;

                if (peer == nullptr && cmd_type != RUdpProtocolCommand::CONNECT)
                    break;

                auto cmd_body = std::vector<uint8_t>{current_data, end};

                if (cmd_type == RUdpProtocolCommand::ACKNOWLEDGE) {
                    core::LOG_DEBUG_VA("command was received: ACKNOWLEDGE ({0})", cmd->header.reliable_sequence_number);

                    auto disconnect = [this, checksum](std::shared_ptr<Peer> &peer) {
                        Disconnect(peer, peer->event_data(), checksum);
                    };

                    if (protocol_->HandleAcknowledge(event, peer, cmd, service_time_, disconnect) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::CONNECT) {
                    core::LOG_DEBUG_VA("command was received: CONNECT ({0})", cmd->header.reliable_sequence_number);

                    if (peer) {
                        IS_EVENT_AVAILABLE()
                    }

                    auto duplicate_peers = 0;

                    for (auto &p : peers_) {
                        auto &net = p->net();

                        if (net->StateIs(RUdpPeerState::DISCONNECTED)) {
                            peer = p;
                            break;
                        }
                        else if (net->StateIs(RUdpPeerState::CONNECTING) && p->Address() == received_address_) {
                            if (p->connect_id() == cmd->connect.connect_id) {
                                peer = nullptr;
                                break;
                            }

                            ++duplicate_peers;
                        }
                    }

                    if (peer == nullptr || duplicate_peers >= duplicate_peers_) {
                        IS_EVENT_AVAILABLE()
                    }

                    protocol_->HandleConnect(peer, header, cmd, received_address_, host_incoming_bandwidth_,
                                             host_outgoing_bandwidth_);
                }
                else if (cmd_type == RUdpProtocolCommand::VERIFY_CONNECT) {
                    core::LOG_DEBUG_VA("command was received: VERIFY_CONNECT ({0})", cmd->header.reliable_sequence_number);

                    if (protocol_->HandleVerifyConnect(event, peer, cmd) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::DISCONNECT) {
                    core::LOG_DEBUG_VA("command was received: DISCONNECT ({0})", cmd->header.reliable_sequence_number);

                    if (protocol_->HandleDisconnect(peer, cmd) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::PING) {
                    core::LOG_DEBUG_VA("command was received: PING ({0})", cmd->header.reliable_sequence_number);

                    if (protocol_->HandlePing(peer) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::SEND_RELIABLE) {
                    core::LOG_DEBUG_VA("command was received: SEND_RELIABLE ({0})", cmd->header.reliable_sequence_number);
                    core::LOG_DEBUG_VA("received data: {0}", std::string{cmd_body.begin(), cmd_body.end()});

                    auto &net = peer->net();

                    if (peer->ExceedsChannelCount(cmd->header.channel_id) ||
                        (net->StateIsNot(RUdpPeerState::CONNECTED) &&
                         net->StateIsNot(RUdpPeerState::DISCONNECT_LATER))) {
                        IS_EVENT_AVAILABLE()
                    }

                    auto data_length = cmd->send_reliable.data_length;
                    auto pos         = current_data + data_length;

                    if (/*(data_length > HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE) ||*/
                        pos < received_data_->begin() || pos > received_data_->begin() + received_data_length_) {
                        IS_EVENT_AVAILABLE()
                    }

                    if (protocol_->HandleSendReliable(peer, cmd, cmd_body, data_length,
                                                      static_cast<uint16_t>(SegmentFlag::RELIABLE),
                                                      0) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::SEND_UNRELIABLE) {
                    core::LOG_DEBUG_VA("command was received: SEND_UNRELIABLE ({0})", cmd->header.reliable_sequence_number);

                    // TODO: add implementation
                    // if (protocol_->HandleSendUnreliable(peer, cmd, current_data))
                    //     IS_EVENT_AVAILABLE()
                }
                else if (cmd_type == RUdpProtocolCommand::SEND_UNSEQUENCED) {
                    core::LOG_DEBUG_VA("command was received: SEND_UNSEQUENCED ({0})", cmd->header.reliable_sequence_number);

                    // TODO: add implementation
                    // if (protocol_->HandleSendUnsequenced(peer, cmd, current_data))
                    //     IS_EVENT_AVAILABLE()
                }
                else if (cmd_type == RUdpProtocolCommand::SEND_FRAGMENT) {
                    core::LOG_DEBUG_VA("command was received: SEND_FRAGMENT ({0})", cmd->header.reliable_sequence_number);

                    auto &net = peer->net();

                    if (peer->ExceedsChannelCount(cmd->header.channel_id) ||
                        (net->StateIsNot(RUdpPeerState::CONNECTED) &&
                         net->StateIsNot(RUdpPeerState::DISCONNECT_LATER))) {
                        IS_EVENT_AVAILABLE()
                    }

                    auto data_length = cmd->send_reliable.data_length;
                    auto pos         = current_data + data_length;

                    if (/*(data_length > HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE) ||*/
                        pos < received_data_->begin() || pos > received_data_->begin() + received_data_length_) {
                        IS_EVENT_AVAILABLE()
                    }

                    if (protocol_->HandleSendFragment(peer, cmd, cmd_body,
                                                      static_cast<uint16_t>(SegmentFlag::RELIABLE)) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::BANDWIDTH_LIMIT) {
                    core::LOG_DEBUG_VA("command was received: BANDWIDTH_LIMIT ({0})", cmd->header.reliable_sequence_number);

                    if (protocol_->HandleBandwidthLimit(peer, cmd, current_data) == Error::ERROR) {
                        IS_EVENT_AVAILABLE()
                    }
                }
                else if (cmd_type == RUdpProtocolCommand::THROTTLE_CONFIGURE) {
                    core::LOG_DEBUG_VA("command was received: THROTTLE_CONFIGURE ({0})", cmd->header.reliable_sequence_number);

                    // TODO: add implementation
                    // if (protocol_->HandleThrottleConfigure(peer, cmd))
                    //     IS_EVENT_AVAILABLE()
                }
                else if (cmd_type == RUdpProtocolCommand::SEND_UNRELIABLE_FRAGMENT) {
                    core::LOG_DEBUG_VA("command was received: SEND_UNRELIABLE_FRAGMENT ({0})",
                                 cmd->header.reliable_sequence_number);

                    // TODO: add implementation
                    // if (protocol_->HandleSendUnreliableFragment(peer, cmd, current_data))
                    //     IS_EVENT_AVAILABLE()
                }
                else {
                    IS_EVENT_AVAILABLE()
                }

                if (peer && (cmd->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE)) != 0) {
                    if (!(flags & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME)))
                        break;

                    auto &net      = peer->net();
                    auto sent_time = header->sent_time;

                    if (net->StateIs(RUdpPeerState::DISCONNECTING) ||
                        net->StateIs(RUdpPeerState::ACKNOWLEDGING_CONNECT) ||
                        net->StateIs(RUdpPeerState::DISCONNECTED) || net->StateIs(RUdpPeerState::ZOMBIE)) {
                        // DO NOTHING
                    }
                    else if (net->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT)) {
                        if ((cmd->header.command & PROTOCOL_COMMAND_MASK) ==
                            static_cast<uint8_t>(RUdpProtocolCommand::DISCONNECT))
                            peer->QueueAcknowledgement(cmd, sent_time);
                    }
                    else {
                        peer->QueueAcknowledgement(cmd, sent_time);
                    }
                }
            }

            if (event != nullptr && event->TypeIsNot(EventType::NONE))
                return EventStatus::AN_EVENT_OCCURRED;
        }

        return EventStatus::NO_EVENT_OCCURRED;
    }

    void
    PeerPod::RequestPeerRemoval(size_t peer_idx, const std::shared_ptr<Peer> &peer, ChecksumCallback checksum)
    {
        std::shared_ptr<Segment> segment =
            std::make_shared<Segment>(nullptr, static_cast<uint32_t>(SegmentFlag::RELIABLE));

        segment->AddSysMsg(core::SysMsg::REMOVE_PEER);
        segment->AddPeerIdx(peer_idx);

        peer->Send(core::SysCh::CONFIG, segment, checksum);
    }

    EventStatus
    PeerPod::SendOutgoingCommands(const std::unique_ptr<Event> &event, uint32_t service_time, bool check_for_timeouts,
                                  ChecksumCallback checksum)
    {
        auto header_data = std::vector<uint8_t>(sizeof(ProtocolHeader) + sizeof(uint32_t), 0);
        auto header      = reinterpret_cast<ProtocolHeader *>(&(header_data.at(0)));

        protocol_->continue_sending(true);

        while (protocol_->continue_sending()) {
            protocol_->continue_sending(false);

            for (auto &peer : peers_) {
                auto &net = peer->net();

                if (net->StateIs(RUdpPeerState::DISCONNECTED) || net->StateIs(RUdpPeerState::ZOMBIE))
                    continue;

                protocol_->chamber()->header_flags(0);
                protocol_->chamber()->command_count(0);
                protocol_->chamber()->buffer_count(1);
                protocol_->chamber()->segment_size(sizeof(ProtocolHeader));

                //  ACKを返す
                // --------------------------------------------------

                if (peer->AcknowledgementExists())
                    protocol_->SendAcknowledgements(peer);

                //  タイムアウト処理
                // --------------------------------------------------

                auto &cmd_pod = peer->command_pod();

                if (check_for_timeouts && cmd_pod->SentReliableCommandExists() &&
                    UDP_TIME_GREATER_EQUAL(service_time, cmd_pod->NextTimeout()) &&
                    cmd_pod->Timeout(peer->net(), service_time)) {
                    IS_EVENT_TYPE_NONE()
                }

                //  送信バッファに Reliable Command を転送する
                // --------------------------------------------------

                if ((cmd_pod->OutgoingReliableCommandNotExists() ||
                     protocol_->SendReliableOutgoingCommands(peer, service_time)) &&
                    cmd_pod->SentReliableCommandNotExists() && peer->ExceedsPingInterval(service_time) &&
                    peer->HasEnoughSpace(protocol_->chamber()->segment_size())) {
                    peer->Ping();

                    // ping コマンドをバッファに転送
                    protocol_->SendReliableOutgoingCommands(peer, service_time);
                }

                //  送信バッファに Unreliable Command を転送する
                // --------------------------------------------------

                if (cmd_pod->OutgoingUnreliableCommandExists())
                    protocol_->SendUnreliableOutgoingCommands(peer, service_time);

                if (protocol_->chamber()->command_count() == 0)
                    continue;

                auto drop_sent_time = false;

                if (net->segment_loss_epoch() == 0) {
                    net->segment_loss_epoch(service_time);
                }
                else if (net->ExceedsSegmentLossInterval(service_time) && net->segments_sent() > 0) {
                    net->CalculateSegmentLoss(service_time);
                }

                if (protocol_->chamber()->header_flags() & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME)) {
                    header->sent_time = htons(service_time & 0xFFFF);
                    // protocol_->chamber()->set_data_length(sizeof(ProtocolHeader));
                }
                else {
                    drop_sent_time = true;
                }

                auto should_compress = false;

                if (compressor_->CanCompress()) {
                    // ...
                }

                if (peer->outgoing_peer_id() < PROTOCOL_MAXIMUM_PEER_ID) {
                    auto header_flags = protocol_->chamber()->header_flags();
                    header_flags |= peer->outgoing_session_id()
                                    << static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT);
                    protocol_->chamber()->header_flags(header_flags);
                }

                header->peer_id = htons(peer->outgoing_peer_id() | protocol_->chamber()->header_flags());

                if (checksum != nullptr) {
                    // ...
                }

                if (should_compress) {
                    // ...
                }

                // ⚠️
                // buffers_[0]には必ずヘッダが設定される。なので、_buffersは以下の構造となる
                //
                // buffers_[0]: ヘッダ
                // buffers_[1]: コマンド
                // buffers_[2]: コマンド
                // buffers_[n]: コマンド
                protocol_->chamber()->SetHeader(header_data, drop_sent_time);

                net->last_send_time(service_time);

                auto sent_length = conn_->Send(peer->Address(), protocol_->chamber());

                cmd_pod->RemoveSentUnreliableCommands();

                if (sent_length < 0)
                    return EventStatus::ERROR;

                total_sent_data_ += sent_length;

                ++total_sent_segments_;
            }
        }

        return EventStatus::NO_EVENT_OCCURRED;
    }
} // namespace rudp
