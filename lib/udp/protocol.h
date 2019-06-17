#ifndef P2P_TECHDEMO_LIB_UDP_PROTOCOL_H
#define P2P_TECHDEMO_LIB_UDP_PROTOCOL_H

#include <array>

constexpr uint8_t PROTOCOL_COMMAND_NONE = 0;
constexpr uint8_t PROTOCOL_COMMAND_ACKNOWLEDGE = 1;
constexpr uint8_t PROTOCOL_COMMAND_CONNECT = 2;
constexpr uint8_t PROTOCOL_COMMAND_VERIFY_CONNECT = 3;
constexpr uint8_t PROTOCOL_COMMAND_DISCONNECT = 4;
constexpr uint8_t PROTOCOL_COMMAND_PING = 5;
constexpr uint8_t PROTOCOL_COMMAND_SEND_RELIABLE = 6;
constexpr uint8_t PROTOCOL_COMMAND_SEND_UNRELIABLE = 7;
constexpr uint8_t PROTOCOL_COMMAND_SEND_FRAGMENT = 8;
constexpr uint8_t PROTOCOL_COMMAND_SEND_UNSEQUENCED = 9;
constexpr uint8_t PROTOCOL_COMMAND_BANDWIDTH_LIMIT = 10;
constexpr uint8_t PROTOCOL_COMMAND_THROTTLE_CONFIGURE = 11;
constexpr uint8_t PROTOCOL_COMMAND_SEND_UNRELIABLE_FRAGMENT = 12;
constexpr uint8_t PROTOCOL_COMMAND_COUNT = 13;
constexpr uint8_t PROTOCOL_COMMAND_MASK = 0x0F;
constexpr uint8_t PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE = (1 << 7);
constexpr uint8_t PROTOCOL_COMMAND_FLAG_UNSEQUENCED = (1 << 6);
constexpr uint16_t PROTOCOL_HEADER_FLAG_COMPRESSED = (1 << 14);
constexpr uint16_t PROTOCOL_HEADER_FLAG_SENT_TIME = (1 << 15);
constexpr uint16_t PROTOCOL_HEADER_FLAG_MASK = PROTOCOL_HEADER_FLAG_COMPRESSED | PROTOCOL_HEADER_FLAG_SENT_TIME;
constexpr uint16_t PROTOCOL_HEADER_SESSION_MASK = (3 << 12);
constexpr uint8_t PROTOCOL_HEADER_SESSION_SHIFT = 12;
constexpr uint16_t PROTOCOL_MINIMUM_CHANNEL_COUNT = 1;
constexpr uint16_t PROTOCOL_MINIMUM_MTU = 576;
constexpr uint16_t PROTOCOL_MINIMUM_WINDOW_SIZE = 4096;
constexpr uint16_t PROTOCOL_MAXIMUM_CHANNEL_COUNT = 255;
constexpr uint16_t PROTOCOL_MAXIMUM_MTU = 4096;
constexpr uint16_t PROTOCOL_MAXIMUM_PACKET_COMMANDS = 32;
constexpr uint16_t PROTOCOL_MAXIMUM_PEER_ID = 0xFFF;
constexpr int PROTOCOL_MAXIMUM_WINDOW_SIZE = 65536;
constexpr int PROTOCOL_FRAGMENT_COUNT = 1024 * 1024;

enum class UdpProtocolCommandFlag : uint32_t
{
    ACKNOWLEDGE = (1u << 7u),
    UNSEQUENCED = (1u << 6u)
};

using UdpProtocolHeader = struct UdpProtocolHeader
{
    uint16_t peer_id;
    uint16_t sent_time;
};

using UdpProtocolCommandHeader = struct UdpProtocolCommandHeader
{
    uint8_t command;
    uint8_t channel_id;
    uint16_t reliable_sequence_number;
};

using UdpProtocolAcknowledge = struct UdpProtocolAcknowledge
{
    UdpProtocolCommandHeader header;
    uint16_t received_reliable_sequence_number;
    uint16_t received_sent_time;
};

using UdpProtocolConnect = struct UdpProtocolConnect
{
    UdpProtocolCommandHeader header;
    uint32_t outgoing_peer_id;
    uint8_t incoming_session_id;
    uint8_t outgoing_session_id;
    uint32_t mtu;
    uint32_t window_size;
    uint32_t channel_count;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
    uint32_t connect_id;
    uint32_t data;
};

using UdpProtocolVerifyConnect = struct UdpProtocolVerifyConnect
{
    UdpProtocolCommandHeader header;
    uint16_t outgoing_peer_id;
    uint8_t incoming_session_id;
    uint8_t outgoing_session_id;
    uint32_t mtu;
    uint32_t window_size;
    uint32_t channel_count;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
    uint32_t connect_id;
};

using UdpProtocolDisconnect = struct UdpProtocolDisconnect
{
    UdpProtocolCommandHeader header;
    uint32_t data;
};

using UdpProtocolPing = struct UdpProtocolPing
{
    UdpProtocolCommandHeader header;
};

using UdpProtocolSendReliable = struct UdpProtocolSendReliable
{
    UdpProtocolCommandHeader header;
    uint16_t data_length;
};

using UdpProtocolSendUnreliable = struct UdpProtocolSendUnreliable
{
    UdpProtocolCommandHeader header;
    uint16_t unreliable_sequence_number;
    uint16_t data_length;
};

using UdpProtocolSendUnsequenced = struct UdpProtocolSendUnsequenced
{
    UdpProtocolCommandHeader header;
    uint16_t unsequenced_group;
    uint16_t data_length;
};

using UdpProtocolSendFragment = struct UdpProtocolSendFragment
{
    UdpProtocolCommandHeader header;
    uint16_t start_sequence_number;
    uint16_t data_length;
    uint32_t fragment_count;
    uint32_t fragment_number;
    uint32_t total_length;
    uint32_t fragment_offset;
};

using UdpProtocolBandwidthLimit = struct UdpProtocolBandwidthLimit
{
    UdpProtocolCommandHeader header;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
};

using UdpProtocolThrottleConfigure = struct UdpProtocolThrottleConfigure
{
    UdpProtocolCommandHeader header;
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
};

using UdpProtocolType = union UdpProtocolType
{
    UdpProtocolCommandHeader header;
    UdpProtocolAcknowledge acknowledge;
    UdpProtocolConnect connect;
    UdpProtocolVerifyConnect verify_connect;
    UdpProtocolDisconnect disconnect;
    UdpProtocolPing ping;
    UdpProtocolSendReliable send_reliable;
    UdpProtocolSendUnreliable send_unreliable;
    UdpProtocolSendUnsequenced send_unsequenced;
    UdpProtocolSendFragment send_fragment;
    UdpProtocolBandwidthLimit bandwidth_limit;
    UdpProtocolThrottleConfigure throttle_configure;
};

class UdpProtocol
{
private:
    std::unique_ptr<UdpDispatchQueue> _dispatch_queue;

    std::unique_ptr<UdpChamber> _chamber;

    bool _recalculate_bandwidth_limits;

    size_t _connected_peers;

    size_t _bandwidth_limited_peers;

    uint32_t _bandwidth_throttle_epoch;

public:
    UdpProtocol();

    void send_acknowledgements(std::shared_ptr<UdpPeer> &peer);

    bool
    _udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<UdpPeer> &peer, uint32_t service_time);

    void
    _udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<UdpPeer> &peer, uint32_t service_time);

    void
    _udp_protocol_dispatch_state(std::shared_ptr<UdpPeer> &peer, UdpPeerState state);

    void
    notify_disconnect(std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event);

    bool recalculate_bandwidth_limits();

    void recalculate_bandwidth_limits(bool val);

    bool continue_sending();

    void continue_sending(bool val);

    std::unique_ptr<UdpChamber> &chamber();

    int dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event);

    void udp_peer_reset(const std::shared_ptr<UdpPeer> &peer);

    void udp_peer_reset_queues(const std::shared_ptr<UdpPeer> &peer);

    void increase_bandwidth_limited_peers();

    void increase_connected_peers();

    void decrease_bandwidth_limited_peers();

    void decrease_connected_peers();

    void connect(const std::shared_ptr<UdpPeer> &peer);

    void disconnect(const std::shared_ptr<UdpPeer> &peer);

    void change_state(const std::shared_ptr<UdpPeer> &peer, const UdpPeerState &state);

    size_t connected_peers();

    uint32_t bandwidth_throttle_epoch();

    void bandwidth_throttle_epoch(uint32_t val);

    size_t bandwidth_limited_peers();

    void bandwidth_throttle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth, const std::vector<std::shared_ptr<UdpPeer>> &peers);
};

size_t
udp_protocol_command_size(uint8_t command_number);

#endif // P2P_TECHDEMO_LIB_UDP_PROTOCOL_H
