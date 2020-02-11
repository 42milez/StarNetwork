#ifndef P2P_TECHDEMO_RUDPPROTOCOL_H
#define P2P_TECHDEMO_RUDPPROTOCOL_H

#include <array>

#include "lib/rudp/dispatch/dispatch_hub.h"
#include "lib/rudp/peer/peer.h"
#include "lib/rudp/chamber.h"
#include "lib/rudp/enum.h"
#include "lib/rudp/event.h"

namespace rudp
{
    class RUdpProtocol
    {
    public:
        RUdpProtocol();

        void BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                const std::vector<std::shared_ptr<Peer>> &peers);

        void NotifyDisconnect(std::shared_ptr<Peer> &peer, const std::unique_ptr<Event> &event);

        EventStatus DispatchIncomingCommands(std::unique_ptr<Event> &event);

        Error
        DispatchIncomingReliableCommands(std::shared_ptr<Peer> &peer,
                const std::shared_ptr<RUdpProtocolType> &cmd);

        Error
        DispatchIncomingUnreliableCommands(const std::shared_ptr<Peer> &peer,
                const std::shared_ptr<RUdpProtocolType> &cmd);

        Error HandleAcknowledge(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer,
                const std::shared_ptr<RUdpProtocolType> &cmd, uint32_t service_time,
                std::function<void(std::shared_ptr<Peer> &peer)> disconnect);

        Error
        HandleBandwidthLimit(const std::shared_ptr<Peer> &peer, const std::shared_ptr<RUdpProtocolType> &cmd,
                VecUInt8It &data);

        void HandleConnect(std::shared_ptr<Peer> &peer,
                const ProtocolHeader *header,
                const std::shared_ptr<RUdpProtocolType> &cmd,
                const NetworkConfig &received_address,
                uint32_t host_incoming_bandwidth,
                uint32_t host_outgoing_bandwidth);

        static Error
        HandlePing(const std::shared_ptr<Peer> &peer);

        Error
        HandleSendFragment(std::shared_ptr<Peer> &peer, const std::shared_ptr<RUdpProtocolType> &cmd, VecUInt8 &data, uint16_t flags);

        Error
        HandleSendReliable(std::shared_ptr<Peer> &peer, const std::shared_ptr<RUdpProtocolType> &cmd, VecUInt8 &data,
                uint16_t data_length, uint16_t flags, uint32_t fragment_count);

        Error HandleVerifyConnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer,
                const std::shared_ptr<RUdpProtocolType> &cmd);

        Error HandleDisconnect(std::shared_ptr<Peer> &peer, const std::shared_ptr<RUdpProtocolType> &cmd);

        void ResetPeer(const std::shared_ptr<Peer> &peer);

        void SendAcknowledgements(std::shared_ptr<Peer> &peer);
        bool SendReliableOutgoingCommands(const std::shared_ptr<Peer> &peer, uint32_t service_time);
        void SendUnreliableOutgoingCommands(std::shared_ptr<Peer> &peer, uint32_t service_time);

        inline void PeerOnDisconnect(const std::shared_ptr<Peer> &peer)
        { dispatch_hub_->PurgePeer(peer); }

    public:
        inline bool continue_sending()
        { return chamber_->continue_sending(); };

        inline void continue_sending(bool val)
        { chamber_->continue_sending(val); };

    public:
        inline const std::unique_ptr<Chamber> & chamber()
        { return chamber_; };

    private:
        std::unique_ptr<Chamber> chamber_;
        std::unique_ptr<DispatchHub> dispatch_hub_;

        size_t maximum_waiting_data_;

        uint32_t bandwidth_throttle_epoch_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
