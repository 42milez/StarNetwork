#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/rudp/host.h"
#include "lib/test/util.h"

#include "var.h"

TEST_CASE("host can broadcast to guest peers", "[feature][broadcast]")
{
    // host address
    IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host_v6(host_ip.GetIPv6());
    host_address.port(test::HOST_PORT);

    // host
    rudp::NetworkConfig address;
    address.port(test::HOST_PORT);
    auto host       = std::make_unique<rudp::Host>(address, core::SysCh::MAX, 32, 100, 100);
    auto host_event = std::make_unique<rudp::Event>();

    // guest1
    rudp::NetworkConfig guest1_address;
    guest1_address.port(test::GUEST1_PORT);
    auto guest1       = std::make_unique<rudp::Host>(guest1_address, core::SysCh::MAX, 1, 100, 100);
    auto guest1_event = std::make_unique<rudp::Event>();

    // guest2
    rudp::NetworkConfig guest2_address;
    guest2_address.port(test::GUEST2_PORT);
    auto guest2       = std::make_unique<rudp::Host>(guest2_address, core::SysCh::MAX, 1, 100, 100);
    auto guest2_event = std::make_unique<rudp::Event>();

    guest1->Connect(host_address, core::SysCh::MAX, 0);
    guest1->Service(guest1_event, 0);

    test::wait(
        [&host, &host_event, &guest1, &guest1_event]() {
            host->Service(host_event, 0);
            guest1->Service(guest1_event, 0);
            return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                   (guest1->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
        },
        test::DEFAULT_TIMEOUT);

    guest2->Connect(host_address, core::SysCh::MAX, 0);
    guest2->Service(guest1_event, 0);

    test::wait(
        [&host, &host_event, &guest2, &guest2_event]() {
            host->Service(host_event, 0);
            guest2->Service(guest2_event, 0);
            return (host->PeerState(1) == rudp::RUdpPeerState::CONNECTED) &&
                   (guest2->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
        },
        test::DEFAULT_TIMEOUT);

    std::string msg{"broadcast command from host"};
    auto payload = std::vector<uint8_t>{msg.begin(), msg.end()};
    auto flags   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
    auto segment = std::make_shared<rudp::Segment>(&payload, flags);

    host->Broadcast(core::SysCh::RELIABLE, segment);
    host->Service(host_event, 0);

    test::wait([&host, &host_event, &guest1,
                &guest1_event]() { return guest1->Service(guest1_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
               test::DEFAULT_TIMEOUT);

    REQUIRE((guest1_event->TypeIs(rudp::EventType::RECEIVE) && (guest1_event->DataAsString() == msg)));

    test::wait([&host, &host_event, &guest2,
                &guest2_event]() { return guest2->Service(guest2_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
               test::DEFAULT_TIMEOUT);

    REQUIRE((guest2_event->TypeIs(rudp::EventType::RECEIVE) && (guest2_event->DataAsString() == msg)));
}
