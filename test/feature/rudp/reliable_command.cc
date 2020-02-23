#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/rudp/host.h"
#include "lib/test/util.h"

#include "var.h"

TEST_CASE("guest peer can send reliable command to host peer", "[reliable_command]")
{
    // host address
    IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(test::HOST_PORT);

    // host
    rudp::NetworkConfig address;
    address.port(test::HOST_PORT);
    auto host       = std::make_unique<rudp::Host>(address, rudp::SysCh::MAX, 32, 100, 100);
    auto host_event = std::make_unique<rudp::Event>();

    // guest1
    rudp::NetworkConfig guest1_address;
    guest1_address.port(test::GUEST1_PORT);
    auto guest1       = std::make_unique<rudp::Host>(guest1_address, rudp::SysCh::MAX, 1, 100, 100);
    auto guest1_event = std::make_unique<rudp::Event>();

    // guest2
    rudp::NetworkConfig guest2_address;
    guest2_address.port(test::GUEST2_PORT);
    auto guest2       = std::make_unique<rudp::Host>(guest2_address, rudp::SysCh::MAX, 1, 100, 100);
    auto guest2_event = std::make_unique<rudp::Event>();

    SECTION("guest peer 1 and 2 can send reliable command to host peer")
    {
        //  guest peer 1
        // --------------------------------------------------

        guest1->Connect(host_address, rudp::SysCh::MAX, 0);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event, &guest1, &guest1_event]() {
                host->Service(host_event, 0);
                guest1->Service(guest1_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest1->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg1{"command from guest1"};
        auto data1    = std::vector<uint8_t>{msg1.begin(), msg1.end()};
        auto flags1   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment1 = std::make_shared<rudp::Segment>(data1, flags1);

        guest1->Send(0, rudp::SysCh::RELIABLE, segment1);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event]() { return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
            test::DEFAULT_TIMEOUT);

        REQUIRE((host_event->TypeIs(rudp::EventType::RECEIVE) && host_event->DataAsString() == msg1));

        //  guest peer 2
        // --------------------------------------------------

        guest2->Connect(host_address, rudp::SysCh::MAX, 0);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event, &guest2, &guest2_event]() {
                host->Service(host_event, 0);
                guest2->Service(guest2_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest2->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg2{"command from guest2"};
        auto payload2 = std::vector<uint8_t>{msg2.begin(), msg2.end()};
        auto flags2   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment2 = std::make_shared<rudp::Segment>(payload2, flags2);

        guest2->Send(0, rudp::SysCh::RELIABLE, segment2);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event]() { return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
            test::DEFAULT_TIMEOUT);

        REQUIRE((host_event->TypeIs(rudp::EventType::RECEIVE) && (host_event->DataAsString() == msg2)));
    }

    SECTION("guest peer 1 and 2 can send fragmented reliable command to host peer")
    {
        //  guest peer 1
        // --------------------------------------------------

        guest1->Connect(host_address, rudp::SysCh::MAX, 0);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event, &guest1, &guest1_event]() {
                host->Service(host_event, 0);
                guest1->Service(guest1_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest1->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg1{
            "Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my "
            "unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul."};
        auto payload1 = std::vector<uint8_t>{msg1.begin(), msg1.end()};
        auto flags1   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment1 = std::make_shared<rudp::Segment>(payload1, flags1);

        guest1->Send(0, rudp::SysCh::RELIABLE, segment1);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event]() { return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
            test::DEFAULT_TIMEOUT);

        REQUIRE((host_event->TypeIs(rudp::EventType::RECEIVE) && (host_event->DataAsString() == msg1)));

        //  guest peer 2
        // --------------------------------------------------

        guest2->Connect(host_address, rudp::SysCh::MAX, 0);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event, &guest2, &guest2_event]() {
                host->Service(host_event, 0);
                guest2->Service(guest2_event, 0);
                return (host->PeerState(1) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest2->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg2{
            "Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my "
            "unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be "
            "For "
            "my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the "
            "bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the "
            "Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how "
            "strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of "
            "my "
            "soul."};
        auto payload2 = std::vector<uint8_t>{msg2.begin(), msg2.end()};
        auto flags2   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment2 = std::make_shared<rudp::Segment>(payload2, flags2);

        guest2->Send(0, rudp::SysCh::RELIABLE, segment2);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event]() { return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED; },
            test::DEFAULT_TIMEOUT);

        REQUIRE((host_event->TypeIs(rudp::EventType::RECEIVE) && (host_event->DataAsString() == msg2)));
    }
}
