#ifndef P2P_TECHDEMO_RUDPCOMMON_H
#define P2P_TECHDEMO_RUDPCOMMON_H

#include <functional>
#include <memory>
#include <queue>
#include <variant>
#include <vector>

#include "core/errors.h"
#include "core/io/socket.h"

constexpr uint8_t PROTOCOL_COMMAND_MASK = 0x0F;

constexpr uint16_t PROTOCOL_MINIMUM_CHANNEL_COUNT = 1;

constexpr uint16_t PROTOCOL_MINIMUM_MTU = 576;

constexpr uint16_t PROTOCOL_MINIMUM_WINDOW_SIZE = 4096;

constexpr uint16_t PROTOCOL_MAXIMUM_CHANNEL_COUNT = 255;

constexpr uint16_t PROTOCOL_MAXIMUM_MTU = 4096;

constexpr uint16_t PROTOCOL_MAXIMUM_SEGMENT_COMMANDS = 32;

constexpr uint16_t PROTOCOL_MAXIMUM_PEER_ID = 0xFFF;

constexpr uint32_t PROTOCOL_MAXIMUM_FRAGMENT_COUNT = 1024 * 1024;

constexpr int PROTOCOL_MAXIMUM_WINDOW_SIZE = 65536;

constexpr int PROTOCOL_FRAGMENT_COUNT = 1024 * 1024;

constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_SEGMENT_COMMANDS;

constexpr int HOST_BANDWIDTH_THROTTLE_INTERVAL = 1000;

constexpr int HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE = 32 * 1024 * 1024;

constexpr int HOST_DEFAULT_MAXIMUM_WAITING_DATA = 32 * 1024 * 1024;

constexpr int HOST_DEFAULT_MTU = 1400;

constexpr int PEER_DEFAULT_SEGMENT_THROTTLE = 32;

constexpr int PEER_DEFAULT_ROUND_TRIP_TIME = 500;

constexpr int PEER_FREE_RELIABLE_WINDOWS = 8;

constexpr int PEER_SEGMENT_LOSS_INTERVAL = 10000;

constexpr int PEER_SEGMENT_LOSS_SCALE = 32;

constexpr int PEER_SEGMENT_THROTTLE_ACCELERATION = 2;

constexpr int PEER_SEGMENT_THROTTLE_DECELERATION = 2;

constexpr int PEER_SEGMENT_THROTTLE_INTERVAL = 5000;

constexpr int PEER_SEGMENT_THROTTLE_COUNTER = 7;

constexpr int PEER_SEGMENT_THROTTLE_SCALE = 32;

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

uint32_t
TimeGet();

void
TimeSet(uint32_t new_time_base);

#endif // P2P_TECHDEMO_RUDPCOMMON_H
