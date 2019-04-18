#ifndef P2P_TECHDEMO_LIB_UDP_UDP_H
#define P2P_TECHDEMO_LIB_UDP_UDP_H

#include <functional>
#include <memory>
#include <vector>

#include <uuid/uuid.h>

#include "core/errors.h"
#include "core/io/socket.h"
#include "protocol.h"

struct UdpBuffer;
struct UdpEvent;
struct UdpHost;
struct UdpIncomingCommand;
struct UdpPacket;
struct UdpPeer;

constexpr int HOST_DEFAULT_MAXIMUM_PACKET_SIZE = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MAXIMUM_WAITING_DATA = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MTU = 1400;
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

constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_PACKET_COMMANDS;

using UdpSocket = void *;

using UdpChecksumCallback = void (*)(const UdpBuffer *, size_t buffer_count);
using UdpInterceptCallback = void (*)(UdpHost *host, UdpEvent *event);
using UdpPacketFreeCallback = void (*)(UdpPacket *);

enum class SysCh : int
{
    CONFIG = 1,
    RELIABLE,
    UNRELIABLE,
    MAX
};

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

struct UdpAcknowledgement {
    // EnetListNode acknowledgement_list;
    uint32_t sent_time;
    UdpProtocol command;
};

struct UdpAddress
{
    uint8_t host[16] = { 0 };

    uint16_t port = 0;

    uint8_t wildcard = 0;

    UdpAddress();
};

struct UdpBuffer
{
    void *data;
    size_t data_length;
};

struct UdpChannel
{
    uint16_t outgoing_reliable_sequence_number;
    uint16_t outgoing_unreliable_seaquence_number;
    uint16_t used_reliable_windows;
    std::vector<uint16_t> reliable_windows;
    uint16_t incoming_reliable_sequence_number;
    uint16_t incoming_unreliable_sequence_number;
    std::list<UdpIncomingCommand> incoming_reliable_commands;
    std::list<UdpIncomingCommand> incoming_unreliable_commands;

    UdpChannel();
};

struct UdpCompressor
{
    std::function<size_t(
        const std::vector<UdpBuffer> &in_buffers,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> compress;

    std::function<size_t(
        std::vector<uint8_t> &in_data,
        size_t in_limit,
        std::vector<uint8_t> &out_data,
        size_t out_limit)> decompress;

    std::function<void()> destroy;
};

struct UdpEvent
{
    UdpEventType type;
    UdpPeer *peer;
    uint8_t channel_id;
    uint32_t data;
    UdpPacket *packet;
};

struct UdpHost
{
    std::unique_ptr<Socket> socket;
    uint32_t incoming_bandwidth;
    uint32_t outgoing_bandwidth;
    uint32_t bandwidth_throttle_epoch;
    uint32_t mtu;
    int recalculate_bandwidth_limits;
    std::vector<UdpPeer> peers;
    size_t peer_count;
    SysCh channel_count;
    uint32_t service_time;
//    UdpList dispatch_queue;
    int continue_sending;
    size_t packet_size;
    uint16_t header_flags;
    UdpProtocol commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS];
    size_t command_count;
    std::vector<UdpBuffer> buffers;
    size_t buffer_count;
    UdpChecksumCallback checksum;
    std::shared_ptr<UdpCompressor> compressor;
    uint8_t packet_data[2][PROTOCOL_MAXIMUM_MTU];
    std::unique_ptr<UdpAddress> received_address;
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

    UdpHost(SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth, size_t peer_count);
};

struct UdpIncomingCommand
{
    //ENetListNode incoming_command_list;
    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
    UdpProtocol command;
    uint32_t fragment_count;
    uint32_t fragments_remaining;
    std::vector<uint32_t> fragments;
    std::shared_ptr<UdpPacket> packet;
};

struct UdpOutgoingCommand
{
    //ENetListNode outgoing_command_list;
    uint16_t reliable_sequence_number;
    uint16_t unreliable_sequence_number;
    uint32_t sent_time;
    uint32_t round_trip_timeout;
    uint32_t round_trip_timeout_limit;
    uint32_t fragment_offset;
    uint16_t fragment_length;
    uint16_t send_attempts;
    UdpProtocol command;
    std::shared_ptr<UdpPacket> packet;
};

struct UdpPacket
{
    size_t reference_count;
    uint32_t flags;
    uint8_t data;
    size_t data_length;
    UdpPacketFreeCallback free_callback;
    void *user_data;
};

struct UdpPeer
{
    std::shared_ptr<UdpHost> host;
    uint16_t outgoing_peer_id;
    uuid_t incoming_peer_id;
    uuid_t connect_id;
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

void
udp_address_set_ip(const std::unique_ptr<UdpAddress> &address, const uint8_t *ip, size_t size);

void
udp_host_compress(std::shared_ptr<UdpHost> &host);

void
udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor);

std::shared_ptr<UdpHost>
udp_host_create(const std::unique_ptr<UdpAddress> &address, size_t peer_count, SysCh channel_count, uint32_t in_bandwidth, uint32_t out_bandwidth);

Error
udp_host_connect(std::shared_ptr<UdpHost> &host, const UdpAddress &address, SysCh channel_count, uint32_t data);

#endif // P2P_TECHDEMO_LIB_UDP_UDP_H
