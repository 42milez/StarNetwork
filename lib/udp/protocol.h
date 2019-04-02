#ifndef P2P_TECHDEMO_LIB_UDP_PROTOCOL_H
#define P2P_TECHDEMO_LIB_UDP_PROTOCOL_H

constexpr int PROTOCOL_MAXIMUM_MTU = 4096;
constexpr int PROTOCOL_MAXIMUM_PACKET_COMMANDS = 32;
constexpr int PROTOCOL_MAXIMUM_PEER_ID = 0xFFF;
constexpr int PROTOCOL_MAXIMUM_WINDOW_SIZE = 65536;

using UdpProtocolCommandHeader = struct UdpProtocolCommandHeader {
    uint8_t command;
    uint8_t channel_id;
    uint16_t reliable_sequence_number;
};

using UdpProtocolAcknowledge = struct UdpProtocolAcknowledge {
    UdpProtocolCommandHeader header;
    uint16_t received_reliable_sequence_number;
    uint16_t received_sent_time;
};

using UdpProtocolConnect = struct UdpProtocolConnect {
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
    uint32_t data;
};

using UdpProtocolVerifyConnect = struct UdpProtocolVerifyConnect {
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

using UdpProtocolDisconnect = struct UdpProtocolDisconnect {
    UdpProtocolCommandHeader header;
    uint32_t data;
};

using UdpProtocolPing = struct UdpProtocolPing {
    UdpProtocolCommandHeader header;
};

using UdpProtocolSendReliable = struct UdpProtocolSendReliable {
    UdpProtocolCommandHeader header;
    uint16_t data_length;
};

using UdpProtocolSendUnreliable = struct UdpProtocolSendUnreliable {
    UdpProtocolCommandHeader header;
    uint16_t unreliable_sequence_number;
    uint16_t data_length;
};

using UdpProtocolSendFragment = struct UdpProtocolSendFragment {
    UdpProtocolCommandHeader header;
    uint16_t start_sequence_number;
    uint16_t data_length;
    uint32_t fragment_count;
    uint32_t fragment_number;
    uint32_t total_length;
    uint32_t fragment_offset;
};

using UdpProtocolBandwidthLimit = struct UdpProtocolBandwidthLimit {
    UdpProtocolCommandHeader header;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
};

using UdpProtocolThrottleConfigure = struct UdpProtocolThrottleConfigure {
    UdpProtocolCommandHeader header;
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
};

using UdpProtocol = union UdpProtocol {
    UdpProtocolCommandHeader header;
    UdpProtocolAcknowledge acknowledge;
    UdpProtocolConnect connect;
    UdpProtocolVerifyConnect verify_connect;
    UdpProtocolDisconnect disconnect;
    UdpProtocolPing ping;
    UdpProtocolSendReliable send_reliable;
    UdpProtocolSendUnreliable send_unreliable;
    UdpProtocolSendFragment send_fragment;
    UdpProtocolBandwidthLimit bandwidth_limit;
    UdpProtocolThrottleConfigure throttle_configure;
};

#endif // P2P_TECHDEMO_LIB_UDP_PROTOCOL_H
