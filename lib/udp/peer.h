#ifndef P2P_TECHDEMO_LIB_UDP_PEER_H
#define P2P_TECHDEMO_LIB_UDP_PEER_H

#include <list>
#include <memory>

constexpr int PEER_DEFAULT_PACKET_THROTTLE = 32;
constexpr int PEER_DEFAULT_ROUND_TRIP_TIME = 500;
constexpr int PEER_PACKET_THROTTLE_ACCELERATION = 2;
constexpr int PEER_PACKET_THROTTLE_DECELERATION = 2;
constexpr int PEER_PACKET_THROTTLE_INTERVAL = 5000;
constexpr int PEER_PACKET_THROTTLE_SCALE = 32;
constexpr int PEER_PING_INTERVAL = 500;
constexpr int PEER_RELIABLE_WINDOWS = 16;
constexpr int PEER_TIMEOUT_LIMIT = 32;
constexpr int PEER_TIMEOUT_MINIMUM = 5000;
constexpr int PEER_TIMEOUT_MAXIMUM = 30000;
constexpr int PEER_UNSEQUENCED_WINDOW_SIZE = 1024;
constexpr int PEER_WINDOW_SIZE_SCALE = 64 * 1024;

enum class UdpPeerState : int
{
    DISCONNECTED,
    CONNECTING,
    ACKNOWLEDGING_CONNECT,
    CONNECTION_PENDING,
    CONNECTION_SUCCEEDED,
    CONNECTED,
    DISCONNECT_LATER,
    DISCONNECTING,
    ACKNOWLEDGING_DISCONNECT,
    ZOMBIE
};

using UdpPeer = struct UdpPeer
{
    std::shared_ptr<UdpHost> host;
    uint16_t outgoing_peer_id;
    uint16_t incoming_peer_id;
    uint32_t connect_id;
    uint8_t outgoing_session_id;
    uint8_t incoming_session_id;
    UdpAddress address;
    void *data;
    UdpPeerState state;
    std::vector<UdpChannel> channels;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
    uint32_t incoming_bandwidth_throttle_epoch;
    uint32_t outgoing_bandwidth_throttle_epoch;
    uint32_t incoming_data_total;
    uint32_t outgoing_data_total;
    uint32_t last_send_time;
    uint32_t last_receive_time;
    uint32_t next_timeout;
    uint32_t earliest_timeout;
    uint32_t packet_loss_epoch;
    uint32_t packets_sent;
    uint32_t packets_lost;
    uint32_t packet_loss;
    uint32_t packet_loss_variance;
    uint32_t packet_throttle;
    uint32_t packet_throttle_limit;
    uint32_t packet_throttle_counter;
    uint32_t packet_throttle_epoch;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
    uint32_t packet_throttle_interval;
    uint32_t ping_interval;
    uint32_t timeout_limit;
    uint32_t timeout_minimum;
    uint32_t timeout_maximum;
    uint32_t last_round_trip_time;
    uint32_t last_round_trip_time_variance;
    uint32_t lowest_round_trip_time;
    uint32_t highest_round_trip_time_variance;
    uint32_t round_trip_time;
    uint32_t round_trip_time_variance;
    uint32_t mtu;
    uint32_t window_size;
    uint32_t reliable_data_in_transit;
    uint16_t outgoing_reliable_sequence_number;
    std::list<UdpAcknowledgement> acknowledgements;
    std::list<UdpOutgoingCommand> sent_reliable_commands;
    std::list<UdpOutgoingCommand> sent_unreliable_commands;
    std::list<UdpOutgoingCommand> outgoing_reliable_commands;
    std::list<UdpOutgoingCommand> outgoing_unreliable_commands;
    std::list<UdpOutgoingCommand> dispatched_commands;
    int needs_dispatch;
    uint16_t incoming_unsequenced_group;
    uint16_t outgoing_unsequenced_group;
    uint32_t unsequenced_window[PEER_UNSEQUENCED_WINDOW_SIZE / 32];
    uint32_t event_data;
    size_t total_waiting_data;

    UdpPeer();
};

UdpOutgoingCommand
udp_peer_queue_outgoing_command(UdpPeer &peer, const std::shared_ptr<UdpProtocol> &command, const std::shared_ptr<UdpPacket> &packet, uint32_t offset, uint16_t length);

void
udp_peer_setup_outgoing_command(UdpPeer &peer, UdpOutgoingCommand &outgoing_command);

void
udp_peer_reset(UdpPeer &peer);

void
udp_peer_reset_queues(UdpPeer &peer);

void
udp_peer_on_disconnect(UdpPeer &peer);

#endif // P2P_TECHDEMO_LIB_UDP_PEER_H
