#ifndef P2P_TECHDEMO_RUDPPROTOCOL_H
#define P2P_TECHDEMO_RUDPPROTOCOL_H

#include <array>

#include "RUdpChamber.h"
#include "RUdpDispatchQueue.h"
#include "RUdpEvent.h"
#include "RUdpPeer.h"
#include "RUdpPeerState.h"

class RUdpProtocol
{
public:
    RUdpProtocol();

    void BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                           const std::vector<std::shared_ptr<RUdpPeer>> &peers);

    void ChangeState(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state);

    void Connect(const std::shared_ptr<RUdpPeer> &peer);
    void Disconnect(const std::shared_ptr<RUdpPeer> &peer);
    void NotifyDisconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

    EventStatus DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event);
    void DispatchState(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state);

    void HandleConnect(std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolHeader * header, RUdpProtocolType * cmd);

    void ResetPeer(const std::shared_ptr<RUdpPeer> &peer);
    static void ResetPeerQueues(const std::shared_ptr<RUdpPeer> &peer);

    void SendAcknowledgements(std::shared_ptr<RUdpPeer> &peer);
    bool SendReliableOutgoingCommands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);
    void SendUnreliableOutgoingCommands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

public:
    inline bool continue_sending()
    { return chamber_->continue_sending(); };

    inline void continue_sending(bool val)
    { chamber_->continue_sending(val); };

    inline void decrease_bandwidth_limited_peers()
    { --bandwidth_limited_peers_; };

    inline void decrease_connected_peers()
    { --connected_peers_; };

    inline void increase_bandwidth_limited_peers()
    { ++bandwidth_limited_peers_; };

    inline void increase_connected_peers()
    { ++connected_peers_; };

public:
    inline const std::unique_ptr<RUdpChamber> &chamber()
    { return chamber_; };

private:
    std::unique_ptr<RUdpChamber> chamber_;
    std::unique_ptr<RUdpDispatchQueue> dispatch_queue_;

    size_t bandwidth_limited_peers_;
    size_t connected_peers_;

    uint32_t bandwidth_throttle_epoch_;

    bool recalculate_bandwidth_limits_;
};

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
