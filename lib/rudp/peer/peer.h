#ifndef P2P_TECHDEMO_LIB_RUDP_PEER_PEER_H_
#define P2P_TECHDEMO_LIB_RUDP_PEER_PEER_H_

#include <deque>
#include <list>
#include <memory>
#include <tuple>

#include "lib/core/errors.h"
#include "lib/rudp/chamber.h"
#include "lib/rudp/channel.h"
#include "lib/rudp/checksum.h"
#include "lib/rudp/command/acknowledgement.h"
#include "lib/rudp/command/command.h"
#include "lib/rudp/command/command_pod.h"
#include "lib/rudp/macro.h"
#include "lib/rudp/network_config.h"
#include "lib/rudp/segment.h"
#include "peer_net.h"

namespace rudp
{
    class Peer
    {
      public:
        Peer();

        void
        ClearDispatchedCommandQueue();

        bool
        EventOccur(const NetworkConfig &address, uint8_t session_id);

        bool
        LoadReliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber, uint32_t service_time);

        bool
        LoadUnreliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber);

        void
        Ping();

        std::shared_ptr<Acknowledgement>
        PopAcknowledgement();

        void
        QueueAcknowledgement(const std::shared_ptr<ProtocolType> &cmd, uint16_t sent_time);

        core::Error
        QueueIncomingCommand(const std::shared_ptr<ProtocolType> &cmd, std::vector<uint8_t> &data, uint16_t data_length,
                             uint16_t flags, uint32_t fragment_count, size_t maximum_waiting_data);

        void
        QueueOutgoingCommand(const std::shared_ptr<ProtocolType> &command, const std::shared_ptr<Segment> &segment,
                             uint32_t offset);

        std::tuple<std::shared_ptr<Segment>, uint8_t>
        Receive();

        RUdpProtocolCommand
        RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id);

        void
        Reset(uint16_t peer_idx);

        void
        ResetPeerQueues();

        core::Error
        Send(core::SysCh ch, const std::shared_ptr<Segment> &segment, ChecksumCallback checksum);

        core::Error
        Setup(const NetworkConfig &address, core::SysCh channel_count, uint32_t host_incoming_bandwidth,
              uint32_t host_outgoing_bandwidth, uint32_t data);

        void
        SetupConnectedPeer(const std::shared_ptr<ProtocolType> &cmd, const NetworkConfig &received_address,
                           uint32_t host_incoming_bandwidth, uint32_t host_outgoing_bandwidth, uint32_t channel_count);

        void
        UpdateRoundTripTimeVariance(uint32_t service_time, uint32_t round_trip_time);

        inline bool
        AcknowledgementExists()
        {
            return !acknowledgements_.empty();
        }

        inline const NetworkConfig &
        Address()
        {
            return address_;
        };

        inline void
        Address(const NetworkConfig val)
        {
            address_ = val;
        }

        std::shared_ptr<Channel>
        GetChannel(uint8_t val)
        {
            return channels_.at(val);
        }

        inline bool
        ChannelExists()
        {
            return !channels_.empty();
        }

        inline void
        ClearAcknowledgement()
        {
            acknowledgements_.clear();
        }

        inline void
        ClearChannel()
        {
            channels_.clear();
        }

        inline bool
        Disconnected()
        {
            return net_->StateIs(RUdpPeerState::DISCONNECTED);
        }

        void
        PushIncomingCommandsToDispatchQueue(const std::vector<std::shared_ptr<IncomingCommand>> &commands)
        {
            for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
                dispatched_commands_.push_front((*it));
            }
        }

        inline bool
        DispatchedCommandExists()
        {
            return !dispatched_commands_.empty();
        }

        inline bool
        ExceedsChannelCount(uint8_t val)
        {
            return val >= channels_.size();
        }

        inline bool
        ExceedsPingInterval(uint32_t service_time)
        {
            return UDP_TIME_DIFFERENCE(service_time, last_receive_time_) >= ping_interval_;
        }

        inline bool
        HasEnoughSpace(size_t segment_size)
        {
            return net_->mtu() - segment_size >= sizeof(ProtocolPing);
        }

        inline void
        Reset()
        {
            this->Reset(incoming_peer_id_);
        }

        inline uint8_t
        StateAsNumber()
        {
            return static_cast<uint8_t>(net_->state());
        }

        inline bool
        StateIs(RUdpPeerState val)
        {
            return net_->StateIs(val);
        }

      public:
        inline const std::unique_ptr<CommandPod> &
        command_pod()
        {
            return command_pod_;
        };

        inline uint32_t
        connect_id()
        {
            return connect_id_;
        }

        inline uint32_t
        data()
        {
            return data_;
        }

        inline void
        data(uint32_t val)
        {
            data_ = val;
        }

        inline uint32_t
        event_data()
        {
            return event_data_;
        };

        inline void
        event_data(uint32_t val)
        {
            event_data_ = val;
        };

        inline void
        incoming_session_id(uint8_t val)
        {
            incoming_session_id_ = val;
        }

        inline void
        last_receive_time(uint32_t val)
        {
            last_receive_time_ = val;
        }

        inline bool
        needs_dispatch()
        {
            return needs_dispatch_;
        };

        inline void
        needs_dispatch(bool val)
        {
            needs_dispatch_ = val;
        };

        inline const std::unique_ptr<PeerNet> &
        net()
        {
            return net_;
        };

        inline uint16_t
        outgoing_peer_id()
        {
            return outgoing_peer_id_;
        };

        inline void
        outgoing_peer_id(uint16_t val)
        {
            outgoing_peer_id_ = val;
        }

        inline uint8_t
        outgoing_session_id()
        {
            return outgoing_session_id_;
        };

        inline void
        outgoing_session_id(uint8_t val)
        {
            outgoing_session_id_ = val;
        }

      private:
        std::list<std::shared_ptr<Acknowledgement>> acknowledgements_;
        std::vector<std::shared_ptr<Channel>> channels_;
        std::unique_ptr<CommandPod> command_pod_;
        std::deque<std::shared_ptr<IncomingCommand>> dispatched_commands_;
        std::unique_ptr<PeerNet> net_;
        std::array<uint32_t, PEER_UNSEQUENCED_WINDOW_SIZE / 32> unsequenced_windows_;

        NetworkConfig address_;

        size_t total_waiting_data_;

        uint16_t incoming_peer_id_;
        uint8_t incoming_session_id_;

        uint16_t outgoing_peer_id_;
        uint8_t outgoing_session_id_;

        uint32_t highest_round_trip_time_variance_;
        uint32_t last_receive_time_;
        uint32_t last_round_trip_time_;
        uint32_t last_round_trip_time_variance_;
        uint32_t lowest_round_trip_time_;

        uint32_t connect_id_;
        uint32_t data_;
        uint32_t event_data_;
        uint32_t ping_interval_;

        bool needs_dispatch_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_PEER_PEER_H_
