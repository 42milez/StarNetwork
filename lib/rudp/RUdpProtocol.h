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
private:
    std::unique_ptr<RUdpDispatchQueue> _dispatch_queue;

    std::unique_ptr<UdpChamber> _chamber;

    bool _recalculate_bandwidth_limits;

    size_t _connected_peers;

    size_t _bandwidth_limited_peers;

    uint32_t _bandwidth_throttle_epoch;

public:
    RUdpProtocol();

    void send_acknowledgements(std::shared_ptr<RUdpPeer> &peer);

    bool
    _udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    void
    _udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time);

    void
    _udp_protocol_dispatch_state(std::shared_ptr<RUdpPeer> &peer, RUdpPeerState state);

    void
    notify_disconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event);

    bool recalculate_bandwidth_limits();

    void recalculate_bandwidth_limits(bool val);

    bool continue_sending();

    void continue_sending(bool val);

    std::unique_ptr<UdpChamber> &chamber();

    int dispatch_incoming_commands(std::unique_ptr<RUdpEvent> &event);

    void udp_peer_reset(const std::shared_ptr<RUdpPeer> &peer);

    void udp_peer_reset_queues(const std::shared_ptr<RUdpPeer> &peer);

    void increase_bandwidth_limited_peers();

    void increase_connected_peers();

    void decrease_bandwidth_limited_peers();

    void decrease_connected_peers();

    void connect(const std::shared_ptr<RUdpPeer> &peer);

    void disconnect(const std::shared_ptr<RUdpPeer> &peer);

    void change_state(const std::shared_ptr<RUdpPeer> &peer, const RUdpPeerState &state);

    size_t connected_peers();

    uint32_t bandwidth_throttle_epoch();

    void bandwidth_throttle_epoch(uint32_t val);

    size_t bandwidth_limited_peers();

    void bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth, const std::vector<std::shared_ptr<RUdpPeer>> &peers);
};

#endif // P2P_TECHDEMO_RUDPPROTOCOL_H
