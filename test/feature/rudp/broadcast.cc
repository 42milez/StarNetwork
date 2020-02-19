#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "lib/rudp/host.h"

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

class HostFixture
{
public:
    HostFixture()
    {
        rudp::NetworkConfig address;
        address.port(8888);
        host_ = std::make_unique<rudp::Host>(address, rudp::SysCh::MAX, 32, 100, 100);
        core::Singleton<core::Logger>::Instance().Init("Broadcast Test");
    }

    void
    Broadcast(rudp::SysCh ch, std::shared_ptr<rudp::Segment> &segment)
    { host_->Broadcast(ch, segment); }

    rudp::RUdpPeerState
    PeerState(size_t idx)
    { return host_->PeerState(0); }

    rudp::EventStatus
    Service(std::unique_ptr<rudp::Event> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

private:
    std::shared_ptr<rudp::Host> host_;
};

TEST_CASE_METHOD(HostFixture, "Broadcast", "[broadcast]")
{
    // host
    IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(8888);

    // guest1
    rudp::NetworkConfig guest1_address;
    guest1_address.port(8889);
    auto guest1 = std::make_unique<rudp::Host>(guest1_address, rudp::SysCh::MAX, 32, 100, 100);

    // guest2
    rudp::NetworkConfig guest2_address;
    guest2_address.port(8890);
    auto guest2 = std::make_unique<rudp::Host>(guest2_address, rudp::SysCh::MAX, 32, 100, 100);

    rudp::EventStatus event_status;

    auto host_event = std::make_unique<rudp::Event>();
    auto guest1_event = std::make_unique<rudp::Event>();
    auto guest2_event = std::make_unique<rudp::Event>();

    LOG("==================================================");
    LOG(" Step 1 : Guest 1 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 1 : CONNECT (1)]");

    guest1->Connect(host_address, rudp::SysCh::MAX, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 1 (2)]");

    guest1->Service(guest1_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (3)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 1 (4)]");

    guest1->Service(guest1_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (5)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 1 (6)]");

    guest1->Service(guest1_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (7)]");

    Service(host_event, 0);

    DELAY();

    REQUIRE(PeerState(0) == rudp::RUdpPeerState::CONNECTED);
    REQUIRE(guest1->PeerState(0) == rudp::RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Guest 2 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 2 : CONNECT (1)]");

    guest2->Connect(host_address, rudp::SysCh::MAX, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 2 (2)]");

    guest2->Service(guest2_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (3)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 2 (4)]");

    guest2->Service(guest2_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (5)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST 2 (6)]");

    guest2->Service(guest2_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (7)]");

    Service(host_event, 0);

    DELAY();

    REQUIRE(PeerState(1) == rudp::RUdpPeerState::CONNECTED);
    REQUIRE(guest2->PeerState(0) == rudp::RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 3 : Broadcast from Host");
    LOG("==================================================");

    std::string msg1{"broadcast command from host"};
    auto data1 = std::vector<uint8_t>{msg1.begin(), msg1.end()};
    auto flags1 = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
    auto segment1 = std::make_shared<rudp::Segment>(data1, flags1);

    LOG("");
    LOG("[HOST : BROADCAST (1)]");

    Broadcast(rudp::SysCh::RELIABLE, segment1);

    DELAY();

    LOG("");
    LOG("[HOST (2)]");

    event_status = Service(guest1_event, 0);

    REQUIRE(event_status == rudp::EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[GUEST 1 (3)]");

    event_status = guest1->Service(guest1_event, 0);

    REQUIRE(event_status == rudp::EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(guest1_event->TypeIs(rudp::EventType::RECEIVE));
    REQUIRE(guest1_event->DataAsString() == "broadcast command from host");

    DELAY();

    LOG("");
    LOG("[GUEST 2 (4)]");

    event_status = guest2->Service(guest2_event, 0);

    REQUIRE(event_status == rudp::EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(guest2_event->TypeIs(rudp::EventType::RECEIVE));
    REQUIRE(guest2_event->DataAsString() == "broadcast command from host");

    DELAY();

    LOG("");
    LOG("[HOST (5)]");

    event_status = Service(guest1_event, 0);

    REQUIRE(event_status == rudp::EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[GUEST 1 (6)]");

    event_status = guest1->Service(host_event, 0);

    REQUIRE(event_status == rudp::EventStatus::NO_EVENT_OCCURRED);

    DELAY();

    LOG("");
    LOG("[GUEST 2 (7)]");

    event_status = guest2->Service(guest2_event, 0);

    REQUIRE(event_status == rudp::EventStatus::NO_EVENT_OCCURRED);

    DELAY();
}
