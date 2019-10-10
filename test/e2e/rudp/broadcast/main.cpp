#define CATCH_CONFIG_MAIN

#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/RUdpHost.h"

namespace
{
constexpr auto SLEEP_DURATION = 100 * 1000; // microsecond

void
LOG(const std::string &message)
{ core::Singleton<core::Logger>::Instance().Debug(message); }

void
DELAY()
{ usleep(SLEEP_DURATION); }
}

class Peer2IPv4Fixture
{
public:
    Peer2IPv4Fixture()
    {
        RUdpAddress address;
        address.port(8888);
        host_ = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);
        core::Singleton<core::Logger>::Instance().Init("Broadcast");
    }

    void
    Broadcast(SysCh ch, std::shared_ptr<RUdpSegment> &segment)
    { host_->Broadcast(ch, segment); }

    RUdpPeerState
    PeerState(size_t idx)
    { return host_->PeerState(0); }

    EventStatus
    Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(Peer2IPv4Fixture, "Broadcast", "[IPv4]")
{
    RUdpAddress peer1_address;
    peer1_address.port(8889);
    auto peer1 = std::make_unique<RUdpHost>(peer1_address, SysCh::MAX, 32, 100, 100);

    IpAddress peer2_ip{"::FFFF:127.0.0.1"};
    RUdpAddress peer2_address;
    peer2_address.host(peer2_ip.GetIPv6());
    peer2_address.port(8888);

    auto peer1_event = std::make_unique<RUdpEvent>();
    auto peer2_event = std::make_unique<RUdpEvent>();

    // ==================================================
    //  Step 1 : peer1 sends CONNECT command to peer2
    // ==================================================

    LOG("[PEER 1 : CONNECT (1)]");

    // [Queue] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    peer1->Connect(peer2_address, SysCh::MAX, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (2)]");

    // [Send] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (3)]");

    // [Receive] PROTOCOL_COMMAND_CONNECT
    // [Queue]   PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_VERIFY_CONNECT
    Service(peer2_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (4)]");

    // [Receive] PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_ACKNOWLEDGE
    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (5)]");

    // [Receive] PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    // [Queue]   PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    // [Send]    PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    Service(peer2_event, 0);
    DELAY();

    REQUIRE(peer1->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);

    // ==================================================
    //  Step 2 : Broadcast (from peer1)
    // ==================================================

    std::string msg{"hello"};
    auto data = std::make_shared<std::vector<uint8_t>>(msg.begin(), msg.end());
    auto flags = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE) |
                 static_cast<uint32_t>(RUdpSegmentFlag::NO_ALLOCATE);
    auto segment = std::make_shared<RUdpSegment>(data, flags);

    LOG("");
    LOG("[PEER 1 : BROADCAST (6)]");

    peer1->Broadcast(SysCh::RELIABLE, segment);
    DELAY();

    LOG("");
    LOG("[PEER 1 (7)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (8)]");

    Service(peer2_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (9)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (10)]");

    Service(peer2_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (11)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    REQUIRE(true);

    // ==================================================
    //  Step 3 : Broadcast (from peer2 )
    // ==================================================

    // ...
}
