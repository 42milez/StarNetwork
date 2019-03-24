#ifndef P2P_TECHDEMO_LIB_UDP_UDP_H
#define P2P_TECHDEMO_LIB_UDP_UDP_H

#include <functional>

#include "protocol.h"

namespace udp
{
    struct _UdpBuffer;
    using UdpBuffer = _UdpBuffer;

    struct _UdpEvent;
    using UdpEvent = _UdpEvent;

    struct _UdpHost;
    using UdpHost = _UdpHost;

    struct _UdpPacket;
    using UdpPacket = _UdpPacket;

    struct _UdpPeer;
    using UdpPeer = _UdpPeer;
}


constexpr int PEER_UNSEQUENCED_WINDOW_SIZE = 1024;
constexpr int PEER_RELIABLE_WINDOWS = 16;

constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_PACKET_COMMANDS;

using UdpSocket = void *;

using UdpChecksumCallback = void (*)(const udp::UdpBuffer *, size_t buffer_count);
using UdpInterceptCallback = void (*)(udp::UdpHost *host, udp::UdpEvent *event);
using UdpPacketFreeCallback = void (*)(udp::UdpPacket *);

enum class UdpEventType : int
{
    NONE,
    CONNECT,
    DISCONNECT,
    RECEIVE
};

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

using UdpAddress = struct _UdpAddress
{
    uint8_t host[16];
    uint16_t port;
    uint8_t wildcard;
};

using UdpBuffer = struct _UdpBuffer
{
    void *data;
    size_t data_length;
};

using UdpChannel = struct _UdpChannel
{
    uint16_t outgoing_reliable_sequence_number;
    uint16_t outgoing_unreliable_seaquence_number;
    uint16_t used_reliable_windows;
    uint16_t reliable_windows[PEER_RELIABLE_WINDOWS];
    uint16_t incoming_reliable_sequence_number;
    uint16_t incoming_unreliable_sequence_number;
    uint16_t incoming_reliable_commands;
    uint16_t incoming_unreliable_commands;
};

using UdpCompressor = struct _UdpCompressor
{
    void *context;

    size_t
    (*compress)(void *context, const UdpBuffer *in_buffer, size_t in_buffer_count, size_t in_limit, uint8_t *out_data, size_t out_limit);

    size_t (*decompress)(void *context, const uint8_t *in_data, size_t in_limit, uint8_t *out_data, size_t out_limit);

    void (*destroy)(void *context);
};

using UdpEvent = struct _UdpEvent
{
    UdpEventType type;
    udp::UdpPeer *peer;
    uint8_t channel_id;
    uint32_t data;
    udp::UdpPacket *packet;
};

using UdpHost = struct _UdpHost
{
    UdpSocket socket;
    UdpAddress address;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
    uint32_t bandwidth_throttle_epoch;
    uint32_t mtu;
    uint32_t random_seed;
    int recalculate_bandwidth_limits;
    udp::UdpPeer *peers;
    size_t peer_count;
    size_t channel_limit;
    uint32_t service_time;
//    UdpList dispatch_queue;
    int continue_sending;
    size_t packet_size;
    uint16_t header_flags;
    UdpProtocol commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS];
    size_t command_count;
    UdpBuffer buffers[BUFFER_MAXIMUM];
    size_t buffer_count;
    UdpChecksumCallback checksum;
    UdpCompressor compressor;
    uint8_t packet_data[2][PROTOCOL_MAXIMUM_MTU];
    UdpAddress received_address;
    uint8_t *received_data;
    size_t received_data_length;
    uint32_t total_sent_data;
    uint32_t total_sent_packets;
    uint32_t total_received_data;
    uint32_t total_received_packets;
    UdpInterceptCallback intercept;
    size_t connected_peers;
    size_t bandwidth_limited_peers;
    size_t duplicate_peers;
    size_t maximum_packet_size;
    size_t maximum_waiting_data;
};

using UdpPacket = struct _UdpPacket
{
    size_t reference_count;
    uint32_t flags;
    uint8_t data;
    size_t data_length;
    UdpPacketFreeCallback free_callback;
    void *user_data;
};

using UdpPeer = struct _UdpPeer
{
//    UdpListNode dispatch_list;
    UdpHost *host;
    uint16_t outgoing_peer_id;
    uint16_t incoming_peer_id;
    int32_t connect_id;
    uint8_t outgoing_session_id;
    uint8_t incoming_session_id;
    UdpAddress address;
    void *data;
    UdpPeerState state;
    UdpChannel *channels;
    size_t channel_count;
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
    uint32_t lowest_round_trip_time;
    uint32_t highest_round_trip_time_variance;
    uint32_t round_trip_time;
    uint32_t round_trip_time_variance;
    uint32_t mtu;
    uint32_t window_size;
    uint32_t reliable_data_in_transit;
    uint16_t outgoing_reliable_sequence_number;
//    UdpList acknowledgements;
//    UdpList sent_reliable_commands;
//    UdpList outgoing_reliable_commands;
//    UdpList outgoing_unreliable_commands;
//    UdpList dispatched_commands;
    int needs_dispatch;
    uint16_t incoming_unsequenced_group;
    uint16_t outgoing_unsequenced_group;
    uint32_t unsequenced_window[PEER_UNSEQUENCED_WINDOW_SIZE / 32];
    uint32_t event_data;
    size_t total_waiting_data;
};

#endif // P2P_TECHDEMO_LIB_UDP_UDP_H
