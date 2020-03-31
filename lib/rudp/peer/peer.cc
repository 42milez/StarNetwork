#ifdef __linux__
#include <arpa/inet.h>
#endif

#include <utility>

#include "lib/core/hash.h"
#include "lib/core/singleton.h"
#include "lib/rudp/enum.h"
#include "lib/rudp/macro.h"
#include "lib/rudp/peer/peer.h"

namespace rudp
{
    Peer::Peer()
        : command_pod_(std::make_unique<CommandPod>())
        , connect_id_()
        , data_()
        , event_data_()
        , highest_round_trip_time_variance_()
        , incoming_peer_id_()
        , incoming_session_id_(0xFF)
        , last_receive_time_()
        , last_round_trip_time_(PEER_DEFAULT_ROUND_TRIP_TIME)
        , last_round_trip_time_variance_()
        , lowest_round_trip_time_(PEER_DEFAULT_ROUND_TRIP_TIME)
        , needs_dispatch_()
        , net_(std::make_unique<PeerNet>())
        , outgoing_peer_id_(PROTOCOL_MAXIMUM_PEER_ID)
        , outgoing_session_id_(0xFF)
        , ping_interval_(PEER_PING_INTERVAL)
        , total_waiting_data_()
        , unsequenced_windows_()
    {
    }

    Error
    Peer::Setup(const NetworkConfig &address, core::SysCh channel_count, uint32_t host_incoming_bandwidth,
                uint32_t host_outgoing_bandwidth, uint32_t data)
    {
        for (auto i = 0; i < static_cast<uint32_t>(channel_count); ++i)
            channels_.emplace_back(std::make_shared<Channel>());

        address_ = address;

        connect_id_ = core::Singleton<core::Hash>::Instance().uniqueID();

        net_->Setup();

        std::shared_ptr<ProtocolType> cmd = std::make_shared<ProtocolType>();

        cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::CONNECT) |
                              static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
        cmd->header.channel_id = 0xFF;

        cmd->connect.channel_count = htonl(static_cast<uint32_t>(channel_count));
        cmd->connect.connect_id    = htonl(connect_id_);
        cmd->connect.data          = htonl(data);
        cmd->connect.mtu           = htonl(net_->mtu());
        cmd->connect.window_size   = htonl(net_->window_size());

        cmd->connect.incoming_bandwidth  = htonl(host_incoming_bandwidth);
        cmd->connect.incoming_session_id = incoming_session_id_;

        cmd->connect.outgoing_bandwidth  = htonl(host_outgoing_bandwidth);
        cmd->connect.outgoing_peer_id    = htons(incoming_peer_id_);
        cmd->connect.outgoing_session_id = outgoing_session_id_;

        cmd->connect.segment_throttle_acceleration = htonl(net_->segment_throttle_acceleration());
        cmd->connect.segment_throttle_deceleration = htonl(net_->segment_throttle_deceleration());
        cmd->connect.segment_throttle_interval     = htonl(net_->segment_throttle_interval());

        QueueOutgoingCommand(cmd, nullptr, 0);

        return Error::OK;
    }

    void
    Peer::SetupConnectedPeer(const std::shared_ptr<ProtocolType> &cmd, const NetworkConfig &received_address,
                             uint32_t host_incoming_bandwidth, uint32_t host_outgoing_bandwidth, uint32_t channel_count)
    {
        net_->state(RUdpPeerState::ACKNOWLEDGING_CONNECT);
        connect_id_       = cmd->connect.connect_id;
        address_          = received_address;
        outgoing_peer_id_ = cmd->connect.outgoing_peer_id;
        net_->incoming_bandwidth(cmd->connect.incoming_bandwidth);
        net_->outgoing_bandwidth(cmd->connect.outgoing_bandwidth);
        net_->segment_throttle_interval(cmd->connect.segment_throttle_interval);
        net_->segment_throttle_acceleration(cmd->connect.segment_throttle_acceleration);
        net_->segment_throttle_deceleration(cmd->connect.segment_throttle_deceleration);
        event_data_ = cmd->connect.data;

        auto incoming_session_id =
            cmd->connect.incoming_session_id == 0xFF ? outgoing_session_id_ : cmd->connect.incoming_session_id;
        incoming_session_id =
            (incoming_session_id + 1) & (static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK) >>
                                         static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT));

        if (incoming_session_id == outgoing_session_id_)
            incoming_session_id =
                (incoming_session_id + 1) & (static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK) >>
                                             static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT));

        outgoing_session_id_ = incoming_session_id;

        auto outgoing_session_id =
            cmd->connect.outgoing_session_id == 0xFF ? incoming_session_id_ : cmd->connect.outgoing_session_id;
        outgoing_session_id =
            (outgoing_session_id + 1) & (static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK) >>
                                         static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT));

        if (outgoing_session_id == incoming_session_id)
            outgoing_session_id =
                (outgoing_session_id + 1) & (static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_MASK) >>
                                             static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SESSION_SHIFT));

        incoming_session_id_ = outgoing_session_id;

        for (auto i = 0; i < static_cast<uint32_t>(channel_count); ++i)
            channels_.emplace_back(std::make_shared<Channel>());

        for (auto &ch : channels_)
            ch->Reset();

        auto mtu = cmd->connect.mtu;

        if (mtu < PROTOCOL_MINIMUM_MTU) {
            mtu = PROTOCOL_MINIMUM_MTU;
        }
        else if (mtu > PROTOCOL_MAXIMUM_MTU) {
            mtu = PROTOCOL_MAXIMUM_MTU;
        }

        net_->mtu(mtu);

        if (host_outgoing_bandwidth == 0 && net_->incoming_bandwidth() == 0) {
            net_->window_size(PROTOCOL_MAXIMUM_WINDOW_SIZE);
        }
        else if (host_outgoing_bandwidth == 0 || net_->incoming_bandwidth() == 0) {
            auto ws = (std::max(host_outgoing_bandwidth, net_->incoming_bandwidth()) / PEER_WINDOW_SIZE_SCALE) *
                      PROTOCOL_MINIMUM_WINDOW_SIZE;

            net_->window_size(ws);
        }
        else {
            auto ws = (std::min(host_outgoing_bandwidth, net_->incoming_bandwidth()) / PEER_WINDOW_SIZE_SCALE) *
                      PROTOCOL_MINIMUM_WINDOW_SIZE;

            net_->window_size(ws);
        }

        if (net_->window_size() < PROTOCOL_MINIMUM_WINDOW_SIZE) {
            net_->window_size(PROTOCOL_MINIMUM_WINDOW_SIZE);
        }
        else if (net_->window_size() > PROTOCOL_MAXIMUM_WINDOW_SIZE) {
            net_->window_size(PROTOCOL_MAXIMUM_WINDOW_SIZE);
        }

        auto window_size = 0;

        if (host_incoming_bandwidth == 0) {
            window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
        }
        else {
            window_size = (host_incoming_bandwidth / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;
        }

        if (window_size > cmd->connect.window_size)
            window_size = cmd->connect.window_size;

        if (window_size < PROTOCOL_MINIMUM_WINDOW_SIZE) {
            window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;
        }
        else if (window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE) {
            window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;
        }

        std::shared_ptr<ProtocolType> verify_cmd = std::make_shared<ProtocolType>();

        verify_cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::VERIFY_CONNECT) |
                                     static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
        verify_cmd->header.channel_id                            = 0xFF;
        verify_cmd->verify_connect.outgoing_peer_id              = htons(incoming_peer_id_);
        verify_cmd->verify_connect.incoming_session_id           = incoming_session_id;
        verify_cmd->verify_connect.outgoing_session_id           = outgoing_session_id;
        verify_cmd->verify_connect.mtu                           = htonl(net_->mtu());
        verify_cmd->verify_connect.window_size                   = htonl(window_size);
        verify_cmd->verify_connect.channel_count                 = htonl(channel_count);
        verify_cmd->verify_connect.incoming_bandwidth            = htonl(host_incoming_bandwidth);
        verify_cmd->verify_connect.outgoing_bandwidth            = htonl(host_outgoing_bandwidth);
        verify_cmd->verify_connect.segment_throttle_interval     = htonl(net_->segment_throttle_interval());
        verify_cmd->verify_connect.segment_throttle_acceleration = htonl(net_->segment_throttle_acceleration());
        verify_cmd->verify_connect.segment_throttle_deceleration = htonl(net_->segment_throttle_deceleration());
        verify_cmd->verify_connect.connect_id                    = htonl(connect_id_);

        QueueOutgoingCommand(verify_cmd, nullptr, 0);
    }

    void
    Peer::Ping()
    {
        if (net_->StateIsNot(RUdpPeerState::CONNECTED))
            return;

        std::shared_ptr<ProtocolType> cmd = std::make_shared<ProtocolType>();

        cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::PING) |
                              static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
        cmd->header.channel_id = 0xFF;

        QueueOutgoingCommand(cmd, nullptr, 0);
    }

    void
    Peer::QueueAcknowledgement(const std::shared_ptr<ProtocolType> &cmd, uint16_t sent_time)
    {
        if (cmd->header.channel_id < channels_.size()) {
            auto channel         = channels_.at(cmd->header.channel_id);
            auto reliable_window = cmd->header.reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;
            auto current_window  = channel->incoming_reliable_sequence_number() / PEER_FREE_RELIABLE_WINDOWS;

            if (cmd->header.reliable_sequence_number < channel->incoming_reliable_sequence_number())
                reliable_window += PEER_RELIABLE_WINDOWS;

            if (reliable_window >= current_window + PEER_FREE_RELIABLE_WINDOWS - 1 &&
                reliable_window <= current_window + PEER_FREE_RELIABLE_WINDOWS)
                return;
        }

        auto ack = std::make_shared<Acknowledgement>();
        if (ack == nullptr)
            return;

        command_pod_->IncrementOutgoingDataTotal(sizeof(ProtocolAcknowledge));

        ack->sent_time(sent_time);
        ack->command(*cmd);

        acknowledgements_.push_back(ack);

        core::Singleton<core::Logger>::Instance().Debug("command was queued: ACKNOWLEDGE ({0})",
                                                        cmd->header.reliable_sequence_number);
    }

    namespace
    {
        Error
        DiscardCommand(uint32_t fragment_count)
        {
            if (fragment_count > 0)
                return Error::ERROR;

            return Error::OK;
        }
    } // namespace

    Error
    Peer::QueueIncomingCommand(const std::shared_ptr<ProtocolType> &cmd, std::vector<uint8_t> &data,
                               uint16_t data_length, uint16_t flags, uint32_t fragment_count,
                               size_t maximum_waiting_data)
    {
        if (net_->StateIs(RUdpPeerState::DISCONNECT_LATER))
            return DiscardCommand(fragment_count);

        if (total_waiting_data_ >= maximum_waiting_data)
            return DiscardCommand(fragment_count);

        auto ch         = channels_.at(cmd->header.channel_id);
        auto [_, error] = ch->QueueIncomingCommand(cmd, data, flags, fragment_count);

        if (error == Error::OK)
            total_waiting_data_ += data_length;
        else
            return error;

        return Error::OK;
    }

    // REVIEW: Is the segment necessary as an argument?
    void
    Peer::QueueOutgoingCommand(const std::shared_ptr<ProtocolType> &protocol_type,
                               const std::shared_ptr<Segment> &segment, uint32_t offset)
    {
        std::shared_ptr<OutgoingCommand> outgoing_command = std::make_shared<OutgoingCommand>();

        outgoing_command->command(protocol_type);
        outgoing_command->fragment_offset(offset);

        if (segment) {
            outgoing_command->segment(segment);
            outgoing_command->fragment_length(segment->DataLength());
        }
        else {
            outgoing_command->fragment_length(0);
        }

        auto channel_id = protocol_type->header.channel_id;
        auto cmd_type =
            static_cast<RUdpProtocolCommand>(outgoing_command->command()->header.command & PROTOCOL_COMMAND_MASK);
        std::shared_ptr<Channel> channel = nullptr;

        if (channel_id < channels_.size())
            channel = channels_.at(channel_id);

        command_pod_->SetupOutgoingCommand(outgoing_command, channel);

        if (cmd_type == RUdpProtocolCommand::PING) {
            auto host = address_.host();
            auto port = address_.port();
            core::Singleton<core::Logger>::Instance().Debug(
                "command was queued: PING ({0}) to {1}.{2}.{3}.{4}:{5}",
                outgoing_command->command()->header.reliable_sequence_number, host.at(12), host.at(13), host.at(14),
                host.at(15), port);
        }
        else {
            core::Singleton<core::Logger>::Instance().Debug("command was queued: {0}",
                                                            COMMANDS_AS_STRING.at(static_cast<uint8_t>(cmd_type)));
        }
    }

    std::tuple<std::shared_ptr<Segment>, uint8_t>
    Peer::Receive()
    {
        if (dispatched_commands_.empty())
            return {nullptr, 0};

        auto incoming_command = dispatched_commands_.front();
        auto segment          = incoming_command->segment();

        total_waiting_data_ -= segment->DataLength();

        dispatched_commands_.pop();

        return {segment, incoming_command->header_channel_id()};
    }

    Error
    Peer::Send(core::SysCh ch, const std::shared_ptr<Segment> &segment, ChecksumCallback checksum)
    {
        if (net_->StateIsNot(RUdpPeerState::CONNECTED) || static_cast<uint32_t>(ch) >= channels_.size() ||
            segment->DataLength() > HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE) {
            return Error::ERROR;
        }

        auto fragment_length = net_->mtu() - sizeof(ProtocolHeader) - sizeof(ProtocolSendFragment);

        if (checksum != nullptr)
            fragment_length -= sizeof(uint32_t);

        auto data_length = segment->DataLength();
        auto channel     = channels_.at(static_cast<uint32_t>(ch));
        uint8_t command_number;
        uint16_t start_sequence_number;

        core::Singleton<core::Logger>::Instance().Debug("data length: {0}", data_length);
        core::Singleton<core::Logger>::Instance().Debug("fragment length: {0}", fragment_length);

        if (data_length > fragment_length) {
            auto fragment_count = (data_length + fragment_length - 1) / fragment_length;

            if (fragment_count > PROTOCOL_MAXIMUM_FRAGMENT_COUNT)
                return Error::ERROR;

            // process a segment as unreliable fragment when the frag has RELIABLE and UNRELIABLE_FRAGMENT
            if ((segment->flags() & (static_cast<uint32_t>(SegmentFlag::RELIABLE) |
                                     static_cast<uint32_t>(SegmentFlag::UNRELIABLE_FRAGMENT))) ==
                    static_cast<uint32_t>(SegmentFlag::UNRELIABLE_FRAGMENT) &&
                channel->outgoing_unreliable_sequence_number() < 0xFFFF) {
                command_number        = static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNRELIABLE_FRAGMENT);
                start_sequence_number = channel->outgoing_unreliable_sequence_number() + 1;
                core::Singleton<core::Logger>::Instance().Debug("**********");
            }
            else {
                command_number = static_cast<uint8_t>(RUdpProtocolCommand::SEND_FRAGMENT) |
                                 static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
                start_sequence_number = channel->outgoing_reliable_sequence_number() + 1;
                core::Singleton<core::Logger>::Instance().Debug("**********");
            }

            std::list<std::shared_ptr<OutgoingCommand>> fragments;

            for (auto fragment_number = 0, fragment_offset = 0; fragment_offset < data_length;
                 ++fragment_number, fragment_offset += fragment_length) {
                if (data_length - fragment_offset < fragment_length)
                    fragment_length = data_length - fragment_offset;

                std::shared_ptr<OutgoingCommand> fragment = std::make_shared<OutgoingCommand>();

                if (fragment == nullptr) {
                    // TODO: throw exception
                    // ...
                }

                auto cmd = std::make_shared<ProtocolType>();

                cmd->header.command                      = command_number;
                cmd->header.channel_id                   = static_cast<uint32_t>(ch);
                cmd->send_fragment.start_sequence_number = htons(start_sequence_number);
                cmd->send_fragment.data_length           = htons(fragment_length);
                cmd->send_fragment.fragment_count        = htonl(fragment_count);
                cmd->send_fragment.fragment_number       = htonl(fragment_number);
                cmd->send_fragment.total_length          = htonl(segment->DataLength());
                cmd->send_fragment.fragment_offset       = htonl(fragment_offset);

                fragment->command(cmd);
                fragment->fragment_offset(fragment_offset);
                fragment->fragment_length(fragment_length);
                fragment->segment(segment);

                fragments.push_back(fragment);
            }

            for (auto &f : fragments) {
                command_pod_->SetupOutgoingCommand(f, channels_.at(f->header_channel_id()));
            }

            return Error::OK;
        }

        auto cmd = std::make_shared<ProtocolType>();

        cmd->header.channel_id = static_cast<uint32_t>(ch);

        if ((segment->flags() &
             (static_cast<uint16_t>(SegmentFlag::RELIABLE) | static_cast<uint16_t>(SegmentFlag::UNSEQUENCED))) ==
            static_cast<uint16_t>(SegmentFlag::UNSEQUENCED)) {
            cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNSEQUENCED) |
                                  static_cast<uint8_t>(RUdpProtocolFlag::COMMAND_UNSEQUENCED);
            cmd->send_unsequenced.data_length = htons(segment->DataLength());
        }
        else if (segment->flags() & static_cast<uint16_t>(SegmentFlag::RELIABLE) ||
                 channel->outgoing_unreliable_sequence_number() >= 0xFFFF) {
            cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::SEND_RELIABLE) |
                                  static_cast<uint8_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
            cmd->send_reliable.data_length = htons(segment->DataLength());
        }
        else {
            cmd->header.command              = static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNRELIABLE);
            cmd->send_unreliable.data_length = htons(segment->DataLength());
        }

        QueueOutgoingCommand(cmd, segment, 0);

        return Error::OK;
    }

    std::shared_ptr<Acknowledgement>
    Peer::PopAcknowledgement()
    {
        auto acknowledgement = acknowledgements_.front();

        acknowledgements_.pop_front();

        return acknowledgement;
    }

    void
    Peer::ClearDispatchedCommandQueue()
    {
        std::queue<std::shared_ptr<IncomingCommand>> empty;
        std::swap(dispatched_commands_, empty);
    }

    bool
    Peer::EventOccur(const NetworkConfig &address, uint8_t session_id)
    {
        if (net_->StateIs(RUdpPeerState::DISCONNECTED))
            return false;

        if (net_->StateIs(RUdpPeerState::ZOMBIE))
            return false;

        if (address_ != address)
            return false;

        if (outgoing_peer_id_ < PROTOCOL_MAXIMUM_PEER_ID && session_id != incoming_session_id_)
            return false;

        return true;
    }

    bool
    Peer::LoadReliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber, uint32_t service_time)
    {
        auto can_ping = command_pod_->LoadReliableCommandsIntoChamber(chamber, net_, channels_, service_time);

        return can_ping;
    }

    bool
    Peer::LoadUnreliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber)
    {
        auto disconnected = command_pod_->LoadUnreliableCommandsIntoChamber(chamber, net_);

        return disconnected;
    }

    void
    Peer::Reset(uint16_t peer_idx)
    {
        acknowledgements_.clear();
        channels_.clear();
        command_pod_->Reset();
        net_->Reset();

        std::queue<std::shared_ptr<IncomingCommand>> empty;
        dispatched_commands_.swap(empty);

        address_.Reset();

        connect_id_                       = 0;
        event_data_                       = 0;
        highest_round_trip_time_variance_ = 0;
        incoming_peer_id_                 = peer_idx;
        incoming_session_id_              = 0xFF;
        last_receive_time_                = 0;
        last_round_trip_time_             = PEER_DEFAULT_ROUND_TRIP_TIME;
        last_round_trip_time_variance_    = 0;
        lowest_round_trip_time_           = PEER_DEFAULT_ROUND_TRIP_TIME;
        needs_dispatch_                   = false;
        outgoing_peer_id_                 = PROTOCOL_MAXIMUM_PEER_ID;
        outgoing_session_id_              = 0xFF;
        ping_interval_                    = PEER_PING_INTERVAL;
        total_waiting_data_               = 0;

        memset(&unsequenced_windows_, 0, unsequenced_windows_.size());
    }

    void
    Peer::ResetPeerQueues()
    {
        std::unique_ptr<Channel> channel;

        if (needs_dispatch_)
            needs_dispatch_ = false;

        if (!acknowledgements_.empty())
            acknowledgements_.clear();

        command_pod_->ClearSentReliableCommand();
        command_pod_->ClearSentUnreliableCommand();
        command_pod_->ClearOutgoingReliableCommand();
        command_pod_->ClearOutgoingUnreliableCommand();

        if (!dispatched_commands_.empty()) {
            std::queue<std::shared_ptr<IncomingCommand>> empty;
            std::swap(dispatched_commands_, empty);
        }

        if (!channels_.empty())
            channels_.clear();
    }

    void
    Peer::UpdateRoundTripTimeVariance(uint32_t service_time, uint32_t round_trip_time)
    {
        auto rttv = command_pod_->round_trip_time_variance();
        command_pod_->round_trip_time_variance(rttv - (rttv / 4));

        if (round_trip_time >= command_pod_->round_trip_time()) {
            auto rtt = command_pod_->round_trip_time();
            command_pod_->round_trip_time(rtt + ((round_trip_time - rtt) / 8));

            rttv = command_pod_->round_trip_time_variance();
            command_pod_->round_trip_time_variance(rttv + ((round_trip_time - rtt) / 4));
        }
        else {
            auto rtt = command_pod_->round_trip_time();
            command_pod_->round_trip_time(rtt - ((round_trip_time - rtt) / 8));

            rttv = command_pod_->round_trip_time_variance();
            command_pod_->round_trip_time_variance(rttv + ((round_trip_time - rtt) / 4));
        }

        auto rtt = command_pod_->round_trip_time();
        if (rtt < lowest_round_trip_time_)
            lowest_round_trip_time_ = rtt;

        rttv = command_pod_->round_trip_time_variance();
        if (rttv > highest_round_trip_time_variance_)
            highest_round_trip_time_variance_ = rttv;

        if (net_->segment_throttle_epoch() == 0 ||
            UDP_TIME_DIFFERENCE(service_time, net_->segment_throttle_epoch() >= net_->segment_throttle_interval())) {
            last_round_trip_time_             = lowest_round_trip_time_;
            last_round_trip_time_variance_    = highest_round_trip_time_variance_;
            lowest_round_trip_time_           = command_pod_->round_trip_time();
            highest_round_trip_time_variance_ = command_pod_->round_trip_time_variance();

            net_->segment_throttle_epoch(service_time);
        }
    }

    RUdpProtocolCommand
    Peer::RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id)
    {
        std::shared_ptr<Channel> channel;

        if (channel_id < channels_.size())
            channel = channels_.at(channel_id);
        else
            channel = nullptr;

        return command_pod_->RemoveSentReliableCommand(reliable_sequence_number, channel_id, channel);
    }
} // namespace rudp
