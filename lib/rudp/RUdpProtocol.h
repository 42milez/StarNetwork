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

    void bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                            const std::vector<std::shared_ptr<RUdpPeer>> &peers);

    void change_state(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state);

    void connect(const std::shared_ptr<RUdpPeer> &peer);

    void disconnect(const std::shared_ptr<RUdpPeer> &peer);

    int dispatch_incoming_commands(std::unique_ptr<RUdpEvent> &event);

    void dispatch_state(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state);

    void notify_disconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

    void send_acknowledgements(std::shared_ptr<RUdpPeer> &peer);

    bool send_reliable_outgoing_commands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    void send_unreliable_outgoing_commands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    void udp_peer_reset(const std::shared_ptr<RUdpPeer> &peer);

    void udp_peer_reset_queues(const std::shared_ptr<RUdpPeer> &peer);

public:
    const std::unique_ptr<RUdpChamber> &chamber();

    bool continue_sending();

    void continue_sending(bool val);

    void decrease_bandwidth_limited_peers();

    void decrease_connected_peers();

    void increase_bandwidth_limited_peers();

    void increase_connected_peers();

private:
    std::unique_ptr<RUdpChamber> _chamber;

    std::unique_ptr<RUdpDispatchQueue> _dispatch_queue;

    uint32_t _bandwidth_throttle_epoch;

    size_t _bandwidth_limited_peers;

    size_t _connected_peers;

    bool _recalculate_bandwidth_limits;
};

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
