#ifndef P2P_TECHDEMO_RUDPPROTOCOLTYPE_H
#define P2P_TECHDEMO_RUDPPROTOCOLTYPE_H

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

    using RUdpProtocolPing = struct RUdpProtocolPing
    {
        ProtocolCommandHeader header;
    };

    using RUdpProtocolSendReliable = struct RUdpProtocolSendReliable
    {
        ProtocolCommandHeader header;
        uint16_t data_length;
    };

    using RUdpProtocolSendUnreliable = struct RUdpProtocolSendUnreliable
    {
        ProtocolCommandHeader header;
        uint16_t unreliable_sequence_number;
        uint16_t data_length;
    };

    using RUdpProtocolSendUnsequenced = struct RUdpProtocolSendUnsequenced
    {
        ProtocolCommandHeader header;
        uint16_t unsequenced_group;
        uint16_t data_length;
    };

    using RUdpProtocolSendFragment = struct RUdpProtocolSendFragment
    {
        ProtocolCommandHeader header;
        uint16_t start_sequence_number;
        uint16_t data_length;
        uint32_t fragment_count;
        uint32_t fragment_number;
        uint32_t total_length;
        uint32_t fragment_offset;
    };

    using RUdpProtocolBandwidthLimit = struct RUdpProtocolBandwidthLimit
    {
        ProtocolCommandHeader header;
        uint32_t incoming_bandwidth;
        uint32_t outgoing_bandwidth;
    };

    using RUdpProtocolThrottleConfigure = struct RUdpProtocolThrottleConfigure
    {
        ProtocolCommandHeader header;
        uint32_t segment_throttle_interval;
        uint32_t segment_throttle_acceleration;
        uint32_t segment_throttle_deceleration;
    };

    using RUdpProtocolType = union RUdpProtocolType
    {
        ProtocolCommandHeader header;
        ProtocolAcknowledge acknowledge;
        ProtocolConnect connect;
        ProtocolVerifyConnect verify_connect;
        ProtocolDisconnect disconnect;
        RUdpProtocolPing ping;
        RUdpProtocolSendReliable send_reliable;
        RUdpProtocolSendUnreliable send_unreliable;
        RUdpProtocolSendUnsequenced send_unsequenced;
        RUdpProtocolSendFragment send_fragment;
        RUdpProtocolBandwidthLimit bandwidth_limit;
        RUdpProtocolThrottleConfigure throttle_configure;
    };

    using RUdpProtocolTypeSP = std::shared_ptr<RUdpProtocolType>;

    ProtocolHeader *
    ConvertNetworkByteOrderToHostByteOrder(ProtocolHeader *header);

    RUdpProtocolType *
    ConvertNetworkByteOrderToHostByteOrder(RUdpProtocolType *cmd);
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPPROTOCOLTYPE_H
