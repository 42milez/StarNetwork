#ifndef P2P_TECHDEMO_RUDPPROTOCOL_H
#define P2P_TECHDEMO_RUDPPROTOCOL_H

#include <array>

#include "lib/rudp/dispatch/RUdpDispatchHub.h"
#include "lib/rudp/peer/RUdpPeer.h"
#include "lib/rudp/RUdpChamber.h"
#include "lib/rudp/RUdpEnum.h"
#include "lib/rudp/RUdpEvent.h"

class RUdpProtocol
{
public:
    RUdpProtocol();

    void BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                           const std::vector<std::shared_ptr<RUdpPeer>> &peers);

    void NotifyDisconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

    EventStatus DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event);

    Error HandleAcknowledge(const std::unique_ptr<RUdpEvent> &event, std::shared_ptr<RUdpPeer> &peer,
                            const RUdpProtocolType *cmd, uint32_t service_time,
                            std::function<void(std::shared_ptr<RUdpPeer> &peer)> disconnect);

    Error
    HandleBandwidthLimit(const std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolType *cmd, VecUInt8It &data);

    void HandleConnect(std::shared_ptr<RUdpPeer> &peer,
                       const RUdpProtocolHeader * header,
                       const RUdpProtocolType * cmd,
                       const RUdpAddress &received_address,
                       uint32_t host_incoming_bandwidth,
                       uint32_t host_outgoing_bandwidth);

    static Error
    HandlePing(const std::shared_ptr<RUdpPeer> &peer);

    Error
    HandleSendReliable(const std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolType *cmd, VecUInt8It &data);

    Error HandleVerifyConnect(const std::unique_ptr<RUdpEvent> &event, std::shared_ptr<RUdpPeer> &peer,
                              const RUdpProtocolType *cmd);

    Error HandleDisconnect(std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolType *cmd);

    void ResetPeer(const std::shared_ptr<RUdpPeer> &peer);

    void SendAcknowledgements(std::shared_ptr<RUdpPeer> &peer);
    bool SendReliableOutgoingCommands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);
    void SendUnreliableOutgoingCommands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    inline void PeerOnDisconnect(const std::shared_ptr<RUdpPeer> &peer)
    { dispatch_hub_->PurgePeer(peer); }

public:
    inline bool continue_sending()
    { return chamber_->continue_sending(); };

    inline void continue_sending(bool val)
    { chamber_->continue_sending(val); };

public:
    inline const std::unique_ptr<RUdpChamber> & chamber()
    { return chamber_; };

private:
    std::unique_ptr<RUdpChamber> chamber_;
    std::unique_ptr<RUdpDispatchHub> dispatch_hub_;

    uint32_t bandwidth_throttle_epoch_;
};

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
