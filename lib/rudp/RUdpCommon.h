#ifndef P2P_TECHDEMO_RUDPCOMMON_H
#define P2P_TECHDEMO_RUDPCOMMON_H

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "core/errors.h"
#include "core/io/socket.h"

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
constexpr uint8_t PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE = (1u << 7u);
constexpr uint8_t PROTOCOL_COMMAND_FLAG_UNSEQUENCED = (1u << 6u);
constexpr uint16_t PROTOCOL_HEADER_FLAG_COMPRESSED = (1u << 14u);
constexpr uint16_t PROTOCOL_HEADER_FLAG_SENT_TIME = (1u << 15u);
constexpr uint16_t PROTOCOL_HEADER_FLAG_MASK = PROTOCOL_HEADER_FLAG_COMPRESSED | PROTOCOL_HEADER_FLAG_SENT_TIME;
constexpr uint16_t PROTOCOL_HEADER_SESSION_MASK = (3u << 12u);
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

enum class SysCh : int
{
    CONFIG = 1,
    RELIABLE,
    UNRELIABLE,
    MAX
};

enum class SocketWait : int
{
    NONE = 0,
    SEND = (1u << 0u),
    RECEIVE = (1u << 1u),
    INTERRUPT = (1u << 2u)
};

enum class RUdpProtocolCommandFlag : uint32_t
{
    ACKNOWLEDGE = (1u << 7u),
    UNSEQUENCED = (1u << 6u)
};

using RUdpProtocolHeader = struct RUdpProtocolHeader
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
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
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
    uint32_t packet_throttle_interval;
    uint32_t packet_throttle_acceleration;
    uint32_t packet_throttle_deceleration;
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

#define UDP_TIME_OVERFLOW 86400000 // msec per day (60 sec * 60 sec * 24 h * 1000)

// TODO: 引数が「A is less than B」の順序になるようにする
#define UDP_TIME_LESS(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_GREATER(a, b) ((b) - (a) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_LESS_EQUAL(a, b) (!UDP_TIME_GREATER(a, b))
#define UDP_TIME_GREATER_EQUAL(a, b) (!UDP_TIME_LESS(a, b))
#define UDP_TIME_DIFFERENCE(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW ? (b) - (a) : (a) - (b))

uint32_t
udp_time_get();

void
udp_time_set(uint32_t new_time_base);

#endif // P2P_TECHDEMO_RUDPCOMMON_H
