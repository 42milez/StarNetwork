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

    using RUdpProtocolCommandHeader = struct RUdpProtocolCommandHeader
    {
        uint8_t command;
        uint8_t channel_id;
        uint16_t reliable_sequence_number;
    };

    using RUdpProtocolAcknowledge = struct RUdpProtocolAcknowledge
    {
        RUdpProtocolCommandHeader header;
        uint16_t received_reliable_sequence_number;
        uint16_t received_sent_time;
    };

    using RUdpProtocolConnect = struct RUdpProtocolConnect
    {
        RUdpProtocolCommandHeader header;
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

    using RUdpProtocolVerifyConnect = struct RUdpProtocolVerifyConnect
    {
        RUdpProtocolCommandHeader header;
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

    using RUdpProtocolDisconnect = struct RUdpProtocolDisconnect
    {
        RUdpProtocolCommandHeader header;
        uint32_t data;
    };

    using RUdpProtocolPing = struct RUdpProtocolPing
    {
        RUdpProtocolCommandHeader header;
    };

    using RUdpProtocolSendReliable = struct RUdpProtocolSendReliable
    {
        RUdpProtocolCommandHeader header;
        uint16_t data_length;
    };

    using RUdpProtocolSendUnreliable = struct RUdpProtocolSendUnreliable
    {
        RUdpProtocolCommandHeader header;
        uint16_t unreliable_sequence_number;
        uint16_t data_length;
    };

    using RUdpProtocolSendUnsequenced = struct RUdpProtocolSendUnsequenced
    {
        RUdpProtocolCommandHeader header;
        uint16_t unsequenced_group;
        uint16_t data_length;
    };

    using RUdpProtocolSendFragment = struct RUdpProtocolSendFragment
    {
        RUdpProtocolCommandHeader header;
        uint16_t start_sequence_number;
        uint16_t data_length;
        uint32_t fragment_count;
        uint32_t fragment_number;
        uint32_t total_length;
        uint32_t fragment_offset;
    };

    using RUdpProtocolBandwidthLimit = struct RUdpProtocolBandwidthLimit
    {
        RUdpProtocolCommandHeader header;
        uint32_t incoming_bandwidth;
        uint32_t outgoing_bandwidth;
    };

    using RUdpProtocolThrottleConfigure = struct RUdpProtocolThrottleConfigure
    {
        RUdpProtocolCommandHeader header;
        uint32_t segment_throttle_interval;
        uint32_t segment_throttle_acceleration;
        uint32_t segment_throttle_deceleration;
    };

    using RUdpProtocolType = union RUdpProtocolType
    {
        RUdpProtocolCommandHeader header;
        RUdpProtocolAcknowledge acknowledge;
        RUdpProtocolConnect connect;
        RUdpProtocolVerifyConnect verify_connect;
        RUdpProtocolDisconnect disconnect;
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
