#ifndef P2P_TECHDEMO_RUDPPROTOCOL_H
#define P2P_TECHDEMO_RUDPPROTOCOL_H

#include <array>

#include "RUdpChamber.h"
#include "RUdpDispatchHub.h"
#include "RUdpEvent.h"
#include "RUdpPeer.h"
#include "RUdpPeerState.h"

class RUdpProtocol
{
public:
    RUdpProtocol();

    void BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                           const std::vector<std::shared_ptr<RUdpPeer>> &peers);

    //void PeerOnConnect(const std::shared_ptr<RUdpPeer> &peer);
    //void PeerOnDisconnect(const std::shared_ptr<RUdpPeer> &peer);
    void NotifyDisconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

    EventStatus DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event);

    void HandleConnect(std::shared_ptr<RUdpPeer> &peer,
                       const RUdpProtocolHeader * header,
                       const RUdpProtocolType * cmd,
                       const RUdpAddress &received_address,
                       uint32_t host_incoming_bandwidth,
                       uint32_t host_outgoing_bandwidth);

    Error HandleVerifyConnect(const std::unique_ptr<RUdpEvent> &event, std::shared_ptr<RUdpPeer> &peer,
                              const RUdpProtocolType *cmd);

    Error HandleDisconnect(std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolType *cmd);

    void ResetPeer(const std::shared_ptr<RUdpPeer> &peer);
    static void ResetPeerQueues(const std::shared_ptr<RUdpPeer> &peer);

    void SendAcknowledgements(std::shared_ptr<RUdpPeer> &peer);
    bool SendReliableOutgoingCommands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);
    void SendUnreliableOutgoingCommands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    Error Disconnect(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);
    Error DisconnectNow(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);
    Error DisconnectLater(const std::shared_ptr<RUdpPeer> &peer, uint32_t data);

public:
    inline bool continue_sending()
    { return chamber_->continue_sending(); };

    inline void continue_sending(bool val)
    { chamber_->continue_sending(val); };

//    inline void decrease_bandwidth_limited_peers()
//    { --bandwidth_limited_peers_; };
//
//    inline void decrease_connected_peers()
//    { --connected_peers_; };
//
//    inline void increase_bandwidth_limited_peers()
//    { ++bandwidth_limited_peers_; };
//
//    inline void increase_connected_peers()
//    { ++connected_peers_; };

public:
    inline const std::unique_ptr<RUdpChamber> & chamber()
    { return chamber_; };

private:
    std::unique_ptr<RUdpChamber> chamber_;
    std::unique_ptr<RUdpDispatchHub> dispatch_hub_;

    uint32_t bandwidth_throttle_epoch_;
};

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
