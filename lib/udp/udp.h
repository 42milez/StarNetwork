#ifndef P2P_TECHDEMO_LIB_UDP_UDP_H
#define P2P_TECHDEMO_LIB_UDP_UDP_H

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "core/errors.h"
#include "core/io/socket.h"
#include "command.h"

struct UdpBuffer;
struct UdpIncomingCommand;
struct UdpPeer;

constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_PACKET_COMMANDS;

constexpr int HOST_BANDWIDTH_THROTTLE_INTERVAL = 1000;
constexpr int HOST_DEFAULT_MAXIMUM_PACKET_SIZE = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MAXIMUM_WAITING_DATA = 32 * 1024 * 1024;
constexpr int HOST_DEFAULT_MTU = 1400;

constexpr int PEER_DEFAULT_PACKET_THROTTLE = 32;
constexpr int PEER_DEFAULT_ROUND_TRIP_TIME = 500;
constexpr int PEER_FREE_RELIABLE_WINDOWS = 8;
constexpr int PEER_PACKET_LOSS_INTERVAL = 10000;
constexpr int PEER_PACKET_LOSS_SCALE = 32;
constexpr int PEER_PACKET_THROTTLE_ACCELERATION = 2;
constexpr int PEER_PACKET_THROTTLE_DECELERATION = 2;
constexpr int PEER_PACKET_THROTTLE_INTERVAL = 5000;
constexpr int PEER_PACKET_THROTTLE_COUNTER = 7;
constexpr int PEER_PACKET_THROTTLE_SCALE = 32;
constexpr int PEER_PING_INTERVAL = 500;
constexpr int PEER_RELIABLE_WINDOW_SIZE = 0x1000;
constexpr int PEER_RELIABLE_WINDOWS = 16;
constexpr int PEER_TIMEOUT_LIMIT = 32;
constexpr int PEER_TIMEOUT_MINIMUM = 5000;
constexpr int PEER_TIMEOUT_MAXIMUM = 30000;
constexpr int PEER_UNSEQUENCED_WINDOW_SIZE = 1024;
constexpr int PEER_WINDOW_SIZE_SCALE = 64 * 1024;

constexpr uint32_t SOCKET_WAIT_NONE = 0;
constexpr uint32_t SOCKET_WAIT_SEND = (1u << 0u);
constexpr uint32_t SOCKET_WAIT_RECEIVE = (1u << 1u);
constexpr uint32_t SOCKET_WAIT_INTERRUPT = (1u << 2u);

#define UDP_TIME_OVERFLOW 86400000 // msec per day (60 sec * 60 sec * 24 h * 1000)

// TODO: 引数が「A is less than B」の順序になるようにする
#define UDP_TIME_LESS(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_GREATER(a, b) ((b) - (a) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_LESS_EQUAL(a, b) (!UDP_TIME_GREATER(a, b))
#define UDP_TIME_GREATER_EQUAL(a, b) (!UDP_TIME_LESS(a, b))

#define UDP_TIME_DIFFERENCE(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW ? (b) - (a) : (a) - (b))

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

enum class UdpPacketFlag : uint32_t
{
    // packet must be received by the target peer and
    // resend attempts should be made until the packet is delivered
    RELIABLE = (1u << 0u),

    // packet will not be sequenced with other packets not supported for reliable packets
    UNSEQUENCED = (1u << 1u),

    // packet will not allocate data, and user must supply it instead
    NO_ALLOCATE = (1u << 2u),

    // packet will be fragmented using unreliable (instead of reliable) sends if it exceeds the MTU
    UNRELIABLE_FRAGMENT = (1u << 3u),

    // whether the packet has been sent from all queues it has been entered into
    SENT = (1u << 8u)
};

enum class UdpPeerState : int
{
    DISCONNECTED = 0,
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

enum class UdpSocketWait : int
{
    NONE = 0,
    SEND = (1u << 0u),
    RECEIVE = (1u << 1u),
    INTERRUPT = (1u << 2u)
};

using UdpAcknowledgement = struct UdpAcknowledgement {
    uint32_t sent_time;
    UdpProtocolType command;
};

using UdpAddress = struct UdpAddress
{
    uint8_t host[16] = { 0 };
    uint16_t port = 0;
    uint8_t wildcard = 0;
    UdpAddress();
};

using UdpBuffer = struct UdpBuffer
{
    void *data;
    size_t data_length;
};

using UdpChannel = struct UdpChannel
{
    uint16_t outgoing_reliable_sequence_number;
    uint16_t outgoing_unreliable_seaquence_number;
    uint16_t used_reliable_windows; // 使用中のバッファ（reliable_windows[PEER_RELIABLE_WINDOWS]）
    std::vector<uint16_t> reliable_windows;
    uint16_t incoming_reliable_sequence_number;
    uint16_t incoming_unreliable_sequence_number;
    std::list<UdpIncomingCommand> incoming_reliable_commands;
    std::list<UdpIncomingCommand> incoming_unreliable_commands;

    UdpChannel();
};

using UdpCompressor = struct UdpCompressor
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

    UdpCompressor();
};

using UdpEvent = struct UdpEvent
{
    UdpEventType type;
    uint32_t data;
    std::shared_ptr<UdpPeer> peer;
    std::shared_ptr<UdpPacket> packet;
    uint8_t channel_id;

    UdpEvent();
};

using UdpChecksumCallback = std::function<uint32_t(const std::vector<UdpBuffer> &buffers, size_t buffer_count)>;

void
udp_address_set_ip(UdpAddress &address, const uint8_t *ip, size_t size);

uint32_t
udp_time_get();

void
udp_time_set(uint32_t new_time_base);

class UdpPeerPod
{
private:
    std::vector<std::shared_ptr<UdpPeer>> _peers;

    uint32_t _bandwidth_throttle_epoch;
    size_t _bandwidth_limited_peers;
    size_t _connected_peers;
    size_t _peer_count;

public:
    UdpPeerPod(size_t peer_count);

    std::shared_ptr<UdpPeer> available_peer_exists();

    void bandwidth_throttle(uint32_t _incoming_bandwidth, uint32_t _outgoing_bandwidth, bool &_recalculate_bandwidth_limits);

    int dispatch_incoming_commands(std::unique_ptr<UdpEvent> &event, bool &_recalculate_bandwidth_limits);

    int send_outgoing_commands(std::unique_ptr<UdpEvent> &event, bool check_for_timeouts);
};


class UdpHost
{
private:
    std::queue<std::shared_ptr<UdpPeer>> _dispatch_queue;

    UdpChecksumCallback _checksum;

    UdpBuffer _buffers[BUFFER_MAXIMUM];
    UdpProtocolType _commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS];

    std::vector<uint8_t> _received_data;
    std::vector<std::vector<uint8_t>> _packet_data;

    std::shared_ptr<UdpCompressor> _compressor;

    std::unique_ptr<UdpAddress> _received_address;
    std::unique_ptr<Socket> _socket;

    size_t _packet_size;
    size_t _received_data_length;
    size_t _duplicate_peers;
    size_t _maximum_packet_size;
    size_t _maximum_waiting_data;
    size_t _command_count;
    size_t _buffer_count;

    SysCh _channel_count;

    uint32_t _incoming_bandwidth;
    uint32_t _outgoing_bandwidth;
    uint32_t _mtu;
    uint32_t _service_time;
    uint32_t _total_sent_data;
    uint32_t _total_sent_packets;
    uint32_t _total_received_data;
    uint32_t _total_received_packets;

    bool _recalculate_bandwidth_limits;
    bool _continue_sending;

    uint16_t _header_flags;

    std::unique_ptr<UdpPeerPod> _peer_pod;

public:
    UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth);

    void
    udp_host_compress(std::shared_ptr<UdpHost> &host);

    void
    udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor);

    Error
    udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data);

    int
    udp_host_service(std::unique_ptr<UdpEvent> &event, uint32_t timeout);

    void
    increase_bandwidth_limited_peers();

    void
    increase_connected_peers();

    void
    decrease_bandwidth_limited_peers();

    void
    decrease_connected_peers();
};

class UdpCommandPod
{
private:
    std::list<UdpOutgoingCommand> _outgoing_reliable_commands;
    std::list<UdpOutgoingCommand> _outgoing_unreliable_commands;
    uint32_t _incoming_data_total;
    uint32_t _outgoing_data_total;
    uint16_t _outgoing_reliable_sequence_number;
    uint16_t _incoming_unsequenced_group;
    uint16_t _outgoing_unsequenced_group;

public:
    UdpCommandPod();

    void setup_outgoing_command(UdpOutgoingCommand &outgoing_command);
};

class UdpPeer
{
private:
    UdpHost *_host;
    uint16_t _outgoing_peer_id;
    uint16_t _incoming_peer_id;
    uint32_t _connect_id;
    uint8_t _outgoing_session_id;
    uint8_t _incoming_session_id;
    UdpAddress _address;
    void *_data;
    UdpPeerState _state;
    std::vector<std::shared_ptr<UdpChannel>> _channels;
    uint32_t _incoming_bandwidth;
    uint32_t _outgoing_bandwidth;
    uint32_t _incoming_bandwidth_throttle_epoch;
    uint32_t _outgoing_bandwidth_throttle_epoch;
    uint32_t _last_send_time;
    uint32_t _last_receive_time;
    uint32_t _next_timeout;
    uint32_t _earliest_timeout;
    uint32_t _packet_loss_epoch;
    uint32_t _packets_sent;
    uint32_t _packets_lost;
    uint32_t _packet_loss;
    uint32_t _packet_loss_variance;
    uint32_t _packet_throttle;
    uint32_t _packet_throttle_limit;
    uint32_t _packet_throttle_counter;
    uint32_t _packet_throttle_epoch;
    uint32_t _packet_throttle_acceleration;
    uint32_t _packet_throttle_deceleration;
    uint32_t _packet_throttle_interval;
    uint32_t _ping_interval;
    uint32_t _timeout_limit;
    uint32_t _timeout_minimum;
    uint32_t _timeout_maximum;
    uint32_t _last_round_trip_time;
    uint32_t _last_round_trip_time_variance;
    uint32_t _lowest_round_trip_time;
    uint32_t _highest_round_trip_time_variance;
    uint32_t _round_trip_time;
    uint32_t _round_trip_time_variance;
    uint32_t _mtu;
    uint32_t _window_size;
    uint32_t _reliable_data_in_transit;
    std::list<UdpAcknowledgement> _acknowledgements;
    std::list<UdpOutgoingCommand> _sent_reliable_commands;
    std::list<UdpOutgoingCommand> _sent_unreliable_commands;
    std::queue<UdpIncomingCommand> _dispatched_commands;
    bool _needs_dispatch;
    uint32_t _unsequenced_window[PEER_UNSEQUENCED_WINDOW_SIZE / 32];
    uint32_t _event_data;
    size_t _total_waiting_data;

    std::unique_ptr<UdpCommandPod> _command_pod;

public:
    UdpPeer();

    UdpOutgoingCommand
    queue_outgoing_command(const std::shared_ptr<UdpProtocolType> &command, const std::shared_ptr<UdpPacket> &packet, uint32_t offset, uint16_t length);

    bool is_disconnected();

    Error setup(const UdpAddress &address, SysCh channel_count, uint32_t data, uint32_t in_bandwidth, uint32_t out_bandwidth);
};

class UdpHostCore
{
public:
    Error
    _udp_socket_bind(std::unique_ptr<Socket> &socket, const UdpAddress &address);

    void
    _udp_protocol_change_state(const std::shared_ptr<UdpPeer> &peer, UdpPeerState state);

    void
    _udp_protocol_send_acknowledgements(std::shared_ptr<UdpPeer> &peer);

    int
    _udp_protocol_check_timeouts(std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event);

    bool
    _udp_protocol_send_reliable_outgoing_commands(const std::shared_ptr<UdpPeer> &peer);

    void
    _udp_protocol_send_unreliable_outgoing_commands(std::shared_ptr<UdpPeer> &peer);

    void
    _udp_protocol_remove_sent_unreliable_commands(const std::shared_ptr<UdpPeer> &peer);

    ssize_t
    _udp_socket_send(const UdpAddress &address);

    void
    _udp_protocol_dispatch_state(const std::shared_ptr<UdpPeer> &peer, UdpPeerState state);

    void
    _udp_protocol_notify_disconnect(const std::shared_ptr<UdpPeer> &peer, const std::unique_ptr<UdpEvent> &event);

    bool
    _sending_continues(UdpProtocolType *command,
                       UdpBuffer *buffer,
                       const std::shared_ptr<UdpPeer> &peer,
                       const std::__list_iterator<UdpOutgoingCommand, void *> &outgoing_command);

    std::shared_ptr<UdpPeer>
    _pop_peer_from_dispatch_queue();
};

void
udp_peer_reset(const std::shared_ptr<UdpPeer> &peer);

void
udp_peer_reset_queues(const std::shared_ptr<UdpPeer> &peer);

void
udp_peer_on_connect(const std::shared_ptr<UdpPeer> &peer);

void
udp_peer_on_disconnect(const std::shared_ptr<UdpPeer> &peer);

void
udp_peer_disconnect(std::shared_ptr<UdpPeer> &peer);

std::shared_ptr<UdpPacket>
udp_peer_receive(std::shared_ptr<UdpPeer> &peer, uint8_t &channel_id);

void
udp_peer_ping(std::shared_ptr<UdpPeer> &peer);

#endif // P2P_TECHDEMO_LIB_UDP_UDP_H
