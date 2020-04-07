#ifndef P2P_TECHDEMO_LIB_RUDP_CONST_H_
#define P2P_TECHDEMO_LIB_RUDP_CONST_H_

#include <array>
#include <cstdint>

#include "lib/rudp/protocol/protocol_type.h"

namespace rudp
{
    constexpr std::array<size_t, 13> COMMAND_SIZES{0,
                                                   sizeof(ProtocolAcknowledge),
                                                   sizeof(ProtocolConnect),
                                                   sizeof(ProtocolVerifyConnect),
                                                   sizeof(ProtocolDisconnect),
                                                   sizeof(ProtocolPing),
                                                   sizeof(ProtocolSendReliable),
                                                   sizeof(ProtocolSendUnreliable),
                                                   sizeof(ProtocolSendFragment),
                                                   sizeof(ProtocolSendUnsequenced),
                                                   sizeof(ProtocolBandwidthLimit),
                                                   sizeof(ProtocolThrottleConfigure),
                                                   sizeof(ProtocolSendFragment)};

    constexpr std::array<const char *, 14> COMMANDS_AS_STRING{"NONE",
                                                              "ACKNOWLEDGE",
                                                              "CONNECT",
                                                              "VERIFY_CONNECT",
                                                              "DISCONNECT",
                                                              "PING",
                                                              "SEND_RELIABLE",
                                                              "SEND_UNRELIABLE",
                                                              "SEND_FRAGMENT",
                                                              "SEND_UNSEQUENCED",
                                                              "BANDWIDTH_LIMIT",
                                                              "THROTTLE_CONFIGURE",
                                                              "SEND_UNRELIABLE_FRAGMENT",
                                                              "COUNT"};

    //  Host
    // --------------------------------------------------
    constexpr int HOST_BANDWIDTH_THROTTLE_INTERVAL  = 1000;
    constexpr int HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE = 32 * 1024 * 1024;
    constexpr int HOST_DEFAULT_MAXIMUM_WAITING_DATA = 32 * 1024 * 1024;
    constexpr int HOST_DEFAULT_MTU                  = 1400;

    //  Peer
    // --------------------------------------------------
    constexpr int PEER_DEFAULT_ROUND_TRIP_TIME       = 500;
    constexpr int PEER_DEFAULT_SEGMENT_THROTTLE      = 32;
    constexpr int PEER_FREE_RELIABLE_WINDOWS         = 8;
    constexpr int PEER_PING_INTERVAL                 = 60000; // 1 min
    constexpr int PEER_RELIABLE_WINDOWS              = 16;
    constexpr int PEER_RELIABLE_WINDOW_SIZE          = 0x1000;
    constexpr int PEER_SEGMENT_LOSS_INTERVAL         = 10000;
    constexpr int PEER_SEGMENT_LOSS_SCALE            = 32;
    constexpr int PEER_SEGMENT_THROTTLE_ACCELERATION = 2;
    constexpr int PEER_SEGMENT_THROTTLE_COUNTER      = 7;
    constexpr int PEER_SEGMENT_THROTTLE_DECELERATION = 2;
    constexpr int PEER_SEGMENT_THROTTLE_INTERVAL     = 5000;
    constexpr int PEER_SEGMENT_THROTTLE_SCALE        = 32;
    constexpr int PEER_TIMEOUT_LIMIT                 = 32;
    constexpr int PEER_TIMEOUT_MAXIMUM               = 30000; // 30 sec
    constexpr int PEER_TIMEOUT_MINIMUM               = 5000;  // 5 sec
    constexpr int PEER_UNSEQUENCED_WINDOW_SIZE       = 1024;
    constexpr int PEER_WINDOW_SIZE_SCALE             = 64 * 1024;

    //  Protocol
    // --------------------------------------------------
    constexpr uint8_t PROTOCOL_COMMAND_MASK              = 0x0F;
    constexpr int PROTOCOL_FRAGMENT_COUNT                = 1024 * 1024;
    constexpr uint16_t PROTOCOL_MAXIMUM_CHANNEL_COUNT    = 255;
    constexpr uint32_t PROTOCOL_MAXIMUM_FRAGMENT_COUNT   = 1024 * 1024;
    constexpr uint16_t PROTOCOL_MAXIMUM_MTU              = 4096;
    constexpr uint16_t PROTOCOL_MAXIMUM_PEER_ID          = 0xFFF;
    constexpr uint16_t PROTOCOL_MAXIMUM_SEGMENT_COMMANDS = 32;
    constexpr int PROTOCOL_MAXIMUM_WINDOW_SIZE           = 65536;
    constexpr uint16_t PROTOCOL_MINIMUM_CHANNEL_COUNT    = 1;
    constexpr uint16_t PROTOCOL_MINIMUM_MTU              = 576;
    constexpr uint16_t PROTOCOL_MINIMUM_WINDOW_SIZE      = 4096;

    //  Buffer
    // --------------------------------------------------
    constexpr int BUFFER_MAXIMUM = 1 + 2 * PROTOCOL_MAXIMUM_SEGMENT_COMMANDS;

    //  Socket
    // --------------------------------------------------
    constexpr uint32_t SOCKET_WAIT_INTERRUPT = (1u << 2u);
    constexpr uint32_t SOCKET_WAIT_NONE      = 0;
    constexpr uint32_t SOCKET_WAIT_RECEIVE   = (1u << 1u);
    constexpr uint32_t SOCKET_WAIT_SEND      = (1u << 0u);
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_CONST_H_
