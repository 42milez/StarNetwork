#ifndef P2P_TECHDEMO_LIB_UDP_UDP_H
#define P2P_TECHDEMO_LIB_UDP_UDP_H

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "core/errors.h"
#include "core/io/socket.h"
#include "peer_pod.h"

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

constexpr std::array<size_t, 13> command_sizes{
    0,
    sizeof(UdpProtocolAcknowledge),
    sizeof(UdpProtocolConnect),
    sizeof(UdpProtocolVerifyConnect),
    sizeof(UdpProtocolDisconnect),
    sizeof(UdpProtocolPing),
    sizeof(UdpProtocolSendReliable),
    sizeof(UdpProtocolSendUnreliable),
    sizeof(UdpProtocolSendFragment),
    sizeof(UdpProtocolSendUnsequenced),
    sizeof(UdpProtocolBandwidthLimit),
    sizeof(UdpProtocolThrottleConfigure),
    sizeof(UdpProtocolSendFragment)
};

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

using UdpAcknowledgement = struct UdpAcknowledgement
{
    uint32_t sent_time;
    UdpProtocolType command;
};

using UdpAddress = struct UdpAddress
{
    uint8_t host[16] = {0};

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

class UdpHost
{
private:
    std::vector<uint8_t> _received_data;

    std::vector<std::vector<uint8_t>> _packet_data;

    std::unique_ptr<UdpAddress> _received_address;

    std::unique_ptr<Socket> _socket;

    size_t _received_data_length;

    size_t _duplicate_peers;

    size_t _maximum_packet_size;

    size_t _maximum_waiting_data;

    SysCh _channel_count;

    uint32_t _incoming_bandwidth;

    uint32_t _outgoing_bandwidth;

    uint32_t _mtu;

    uint32_t _service_time;

    std::unique_ptr<UdpPeerPod> _peer_pod;

public:
    UdpHost(const UdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth);

    Error
    udp_host_connect(const UdpAddress &address, SysCh channel_count, uint32_t data);

    int
    udp_host_service(std::unique_ptr<UdpEvent> &event, uint32_t timeout);

    uint32_t service_time();

    Error _udp_socket_bind(std::unique_ptr<Socket> &socket, const UdpAddress &address);

    ssize_t _udp_socket_send(const UdpAddress &address);

    std::shared_ptr<UdpPeer> _pop_peer_from_dispatch_queue();
};

#endif // P2P_TECHDEMO_LIB_UDP_UDP_H
