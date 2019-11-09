#define CATCH_CONFIG_MAIN

#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/RUdpHost.h"

namespace
{
constexpr auto SLEEP_DURATION = 10 * 1000; // microsecond

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
    auto event_status = EventStatus::NO_EVENT_OCCURRED;

    LOG("==================================================");
    LOG(" Step 1 : peer1 sends CONNECT command to peer2");
    LOG("==================================================");

    LOG("");
    LOG("[PEER 1 : CONNECT (1)]");

    peer1->Connect(peer2_address, SysCh::MAX, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (2)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (3)]");

    Service(peer2_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (4)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (5)]");

    Service(peer2_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 1 (6)]");

    peer1->Service(peer1_event, 0);
    DELAY();

    LOG("");
    LOG("[PEER 2 (7)]");

    Service(peer2_event, 0);
    DELAY();

    REQUIRE(peer1->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Broadcast (from peer1)");
    LOG("==================================================");

    std::string msg1{"send broadcast from peer1"};
    auto data1 = std::vector<uint8_t>{msg1.begin(), msg1.end()};
    auto flags1 = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE);
    auto segment1 = std::make_shared<RUdpSegment>(data1, flags1);

    LOG("");
    LOG("[PEER 1 : BROADCAST (1)]");

    peer1->Broadcast(SysCh::RELIABLE, segment1);

    DELAY();

    LOG("");
    LOG("[PEER 1 (2)]");

    event_status = peer1->Service(peer1_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[PEER 2 (3)]");

    event_status = Service(peer2_event, 0);

    REQUIRE(event_status == EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(peer2_event->TypeIs(RUdpEventType::RECEIVE));
    REQUIRE(peer2_event->DataAsString() == "send broadcast from peer1");

    DELAY();

    LOG("");
    LOG("[PEER 1 (4)]");

    event_status = peer1->Service(peer1_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[PEER 2 (5)]");

    event_status = Service(peer2_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("==================================================");
    LOG(" Step 3 : Broadcast (from peer2)");
    LOG("==================================================");

    std::string msg2{"send broadcast from peer2"};
    auto data2 = std::vector<uint8_t>{msg2.begin(), msg2.end()};
    auto flags2 = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE);
    auto segment2 = std::make_shared<RUdpSegment>(data2, flags2);

    LOG("");
    LOG("[PEER 2 : BROADCAST (1)]");

    Broadcast(SysCh::RELIABLE, segment2);
    DELAY();

    LOG("");
    LOG("[PEER 2 (2)]");

    event_status = Service(peer2_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[PEER 1 (3)]");

    event_status = peer1->Service(peer1_event, 0);

    REQUIRE(event_status == EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(peer1_event->TypeIs(RUdpEventType::RECEIVE));
    REQUIRE(peer1_event->DataAsString() == "send broadcast from peer2");

    DELAY();

    LOG("");
    LOG("[PEER 2 (4)]");

    event_status = Service(peer2_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[PEER 1 (5)]");

    event_status = peer1->Service(peer1_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY();
}
