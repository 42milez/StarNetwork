#ifndef P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_H_
#define P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_H_

#include <array>

#include "lib/rudp/chamber.h"
#include "lib/rudp/dispatch/dispatch_hub.h"
#include "lib/rudp/enum.h"
#include "lib/rudp/event.h"
#include "lib/rudp/peer/peer.h"

namespace rudp
{
    class RUdpProtocol
    {
      public:
        RUdpProtocol();

        void
        BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                          const std::vector<std::shared_ptr<Peer>> &peers);

        void
        NotifyDisconnect(std::shared_ptr<Peer> &peer, const std::unique_ptr<Event> &event);

        EventStatus
        DispatchIncomingCommands(std::unique_ptr<Event> &event);

        core::Error
        DispatchIncomingReliableCommands(std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd);

        core::Error
        DispatchIncomingUnreliableCommands(const std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd);

        core::Error
        HandleAcknowledge(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer,
                          const std::shared_ptr<ProtocolType> &cmd, uint32_t service_time,
                          std::function<void(std::shared_ptr<Peer> &peer)> disconnect);

        core::Error
        HandleBandwidthLimit(const std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd,
                             std::vector<uint8_t>::iterator &data);

        void
        HandleConnect(std::shared_ptr<Peer> &peer, const ProtocolHeader *header,
                      const std::shared_ptr<ProtocolType> &cmd, const NetworkConfig &received_address,
                      uint32_t host_incoming_bandwidth, uint32_t host_outgoing_bandwidth);

        static core::Error
        HandlePing(const std::shared_ptr<Peer> &peer);

        core::Error
        HandleSendFragment(std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd,
                           std::vector<uint8_t> &data, uint16_t flags);

        core::Error
        HandleSendReliable(std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd,
                           std::vector<uint8_t> &data, uint16_t data_length, uint16_t flags, uint32_t fragment_count);

        core::Error
        HandleVerifyConnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer,
                            const std::shared_ptr<ProtocolType> &cmd);

        core::Error
        HandleDisconnect(std::shared_ptr<Peer> &peer, const std::shared_ptr<ProtocolType> &cmd);

        void
        ResetPeer(const std::shared_ptr<Peer> &peer);

        void
        SendAcknowledgements(std::shared_ptr<Peer> &peer);

        bool
        SendReliableOutgoingCommands(const std::shared_ptr<Peer> &peer, uint32_t service_time);

        void
        SendUnreliableOutgoingCommands(std::shared_ptr<Peer> &peer, uint32_t service_time);

        inline void
        PeerOnDisconnect(const std::shared_ptr<Peer> &peer)
        {
            dispatch_hub_->PurgePeer(peer);
        }

      public:
        inline bool
        continue_sending()
        {
            return chamber_->continue_sending();
        };

        inline void
        continue_sending(bool val)
        {
            chamber_->continue_sending(val);
        };

      public:
        inline const std::unique_ptr<Chamber> &
        chamber()
        {
            return chamber_;
        };

      private:
        std::unique_ptr<Chamber> chamber_;
        std::unique_ptr<DispatchHub> dispatch_hub_;

        size_t maximum_waiting_data_;

        uint32_t bandwidth_throttle_epoch_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_H_
