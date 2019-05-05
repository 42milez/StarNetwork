#ifndef P2P_TECHDEMO_LIB_UDP_UDP_H
#define P2P_TECHDEMO_LIB_UDP_UDP_H

#include <functional>
#include <memory>
#include <vector>

#include "core/errors.h"
#include "core/io/socket.h"
#include "host.h"
#include "protocol.h"

struct UdpBuffer;
struct UdpIncomingCommand;
struct UdpPeer;

constexpr uint32_t SOCKET_WAIT_NONE = 0;
constexpr uint32_t SOCKET_WAIT_SEND = (1u << 0u);
constexpr uint32_t SOCKET_WAIT_RECEIVE = (1u << 1u);
constexpr uint32_t SOCKET_WAIT_INTERRUPT = (1u << 2u);

#define UDP_TIME_OVERFLOW 86400000
#define UDP_TIME_LESS(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_GREATER_EQUAL(a, b) (!UDP_TIME_LESS(a, b))
#define UDP_TIME_DIFFERENCE(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW ? (b) - (a) : (a) - (b))

enum class UdpEventType : int
{
    NONE,
    CONNECT,
    DISCONNECT,
    RECEIVE
};

using UdpAcknowledgement = struct UdpAcknowledgement {
    // EnetListNode acknowledgement_list;
    uint32_t sent_time;
    UdpProtocol command;
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
    uint16_t used_reliable_windows;
    std::vector<uint16_t> reliable_windows;
    uint16_t incoming_reliable_sequence_number;
    uint16_t incoming_unreliable_sequence_number;
    std::list<UdpIncomingCommand> incoming_reliable_commands;
    std::list<UdpIncomingCommand> incoming_unreliable_commands;

    UdpChannel();
};

using UdpEvent = struct UdpEvent
{
    UdpEventType type;
    uint8_t channel_id;
    uint32_t data;
    std::shared_ptr<UdpPeer> peer;
    std::shared_ptr<UdpPacket> packet;

    UdpEvent();
};

void
udp_address_set_ip(const std::unique_ptr<UdpAddress> &address, const uint8_t *ip, size_t size);

uint32_t
udp_time_get();

void
udp_time_set(uint32_t new_time_base);

#endif // P2P_TECHDEMO_LIB_UDP_UDP_H
