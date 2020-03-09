#ifndef P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_TYPE_H_
#define P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_TYPE_H_

#include <cstdint>
#include <memory>

namespace rudp
{
    using ProtocolHeader = struct ProtocolHeader
    {
        uint16_t peer_id;
        uint16_t sent_time;
    };

    using ProtocolCommandHeader = struct ProtocolCommandHeader
    {
        uint8_t command;
        uint8_t channel_id;
        uint16_t reliable_sequence_number;
    };

    using ProtocolAcknowledge = struct ProtocolAcknowledge
    {
        ProtocolCommandHeader header;
        uint16_t received_reliable_sequence_number;
        uint16_t received_sent_time;
    };

    using ProtocolConnect = struct ProtocolConnect
    {
        ProtocolCommandHeader header;
        uint16_t outgoing_peer_id;
        uint8_t incoming_session_id;
        uint8_t outgoing_session_id;
        uint32_t mtu;
        uint32_t window_size;
        uint32_t channel_count;
        uint32_t incoming_bandwidth;
        uint32_t outgoing_bandwidth;
        uint32_t segment_throttle_interval;
        uint32_t segment_throttle_acceleration;
        uint32_t segment_throttle_deceleration;
        uint32_t connect_id;
        uint32_t data;
    };

    using ProtocolVerifyConnect = struct ProtocolVerifyConnect
    {
        ProtocolCommandHeader header;
        uint16_t outgoing_peer_id;
        uint8_t incoming_session_id;
        uint8_t outgoing_session_id;
        uint32_t mtu;
        uint32_t window_size;
        uint32_t channel_count;
        uint32_t incoming_bandwidth;
        uint32_t outgoing_bandwidth;
        uint32_t segment_throttle_interval;
        uint32_t segment_throttle_acceleration;
        uint32_t segment_throttle_deceleration;
        uint32_t connect_id;
    };

    using ProtocolDisconnect = struct ProtocolDisconnect
    {
        ProtocolCommandHeader header;
        uint32_t data;
    };

    using ProtocolPing = struct ProtocolPing
    {
        ProtocolCommandHeader header;
    };

    using ProtocolSendReliable = struct ProtocolSendReliable
    {
        ProtocolCommandHeader header;
        uint16_t data_length;
    };

    using ProtocolSendUnreliable = struct ProtocolSendUnreliable
    {
        ProtocolCommandHeader header;
        uint16_t unreliable_sequence_number;
        uint16_t data_length;
    };

    using ProtocolSendUnsequenced = struct ProtocolSendUnsequenced
    {
        ProtocolCommandHeader header;
        uint16_t unsequenced_group;
        uint16_t data_length;
    };

    using ProtocolSendFragment = struct ProtocolSendFragment
    {
        ProtocolCommandHeader header;
        uint16_t start_sequence_number;
        uint16_t data_length;
        uint32_t fragment_count;
        uint32_t fragment_number;
        uint32_t total_length;
        uint32_t fragment_offset;
    };

    using ProtocolBandwidthLimit = struct ProtocolBandwidthLimit
    {
        ProtocolCommandHeader header;
        uint32_t incoming_bandwidth;
        uint32_t outgoing_bandwidth;
    };

    using ProtocolThrottleConfigure = struct ProtocolThrottleConfigure
    {
        ProtocolCommandHeader header;
        uint32_t segment_throttle_interval;
        uint32_t segment_throttle_acceleration;
        uint32_t segment_throttle_deceleration;
    };

    using ProtocolType = union ProtocolType {
        ProtocolCommandHeader header;
        ProtocolAcknowledge acknowledge;
        ProtocolConnect connect;
        ProtocolVerifyConnect verify_connect;
        ProtocolDisconnect disconnect;
        ProtocolPing ping;
        ProtocolSendReliable send_reliable;
        ProtocolSendUnreliable send_unreliable;
        ProtocolSendUnsequenced send_unsequenced;
        ProtocolSendFragment send_fragment;
        ProtocolBandwidthLimit bandwidth_limit;
        ProtocolThrottleConfigure throttle_configure;
    };

    using ProtocolTypeSP = std::shared_ptr<ProtocolType>;

    ProtocolHeader *
    ConvertNetworkByteOrderToHostByteOrder(ProtocolHeader *header);

    ProtocolType *
    ConvertNetworkByteOrderToHostByteOrder(ProtocolType *cmd);
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_PROTOCOL_PROTOCOL_TYPE_H_
