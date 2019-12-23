#define CATCH_CONFIG_MAIN

#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/RUdpHost.h"

namespace
{
    void
    LOG(const std::string &message)
    { core::Singleton<core::Logger>::Instance().Debug(message); }

    void
    DELAY_SHORT()
    { usleep(10 * 1000); }

    void
    DELAY_LONG()
    { usleep(100 * 1000); }
}

// テスト内容
// 1. Guest1からHostに接続
// 2. Guest2からHostに接続
// 3. Guest1からHostにデータ送信（Host側で正常に受信できることを検証）
// 4. Guest2からHostにデータ送信（Host側で正常に受信できることを検証）
TEST_CASE("guest sends a reliable command to host", "[reliable command]")
{
    // ToDo: Catch2 の main() に移す
    core::Singleton<core::Logger>::Instance().Init("");

    // host address
    IpAddress host_ip{"::FFFF:127.0.0.1"};
    RUdpAddress host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(10000);

    // host
    RUdpAddress address;
    address.port(10000);
    auto host = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);

    // guest1
    RUdpAddress guest1_address;
    guest1_address.port(10001);
    auto guest1 = std::make_unique<RUdpHost>(guest1_address, SysCh::MAX, 1, 100, 100);

    // guest2
    RUdpAddress guest2_address;
    guest2_address.port(10002);
    auto guest2 = std::make_unique<RUdpHost>(guest2_address, SysCh::MAX, 1, 100, 100);

    EventStatus event_status;

    auto host_event = std::make_unique<RUdpEvent>();
    auto guest1_event = std::make_unique<RUdpEvent>();
    auto guest2_event = std::make_unique<RUdpEvent>();

    LOG("==================================================");
    LOG(" Step 1 : Guest 1 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 1 : CONNECT (1)]");

    guest1->Connect(host_address, SysCh::MAX, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (2)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (4)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (6)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (7)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    REQUIRE(host->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(guest1->PeerState(0) == RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Guest 2 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 2 : CONNECT (1)]");

    guest2->Connect(host_address, SysCh::MAX, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (2)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (4)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (6)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (7)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    REQUIRE(host->PeerState(1) == RUdpPeerState::CONNECTED);
    REQUIRE(guest2->PeerState(0) == RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 3 : Guest 1 sends a reliable command to Host");
    LOG("==================================================");

    std::string msg1{"command from guest1"};
    auto data1 = std::vector<uint8_t>{msg1.begin(), msg1.end()};
    auto flags1 = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE);
    auto segment1 = std::make_shared<RUdpSegment>(data1, flags1);

    Error err;

    LOG("");
    LOG("[GUEST 1 : SEND (1)]");

    err = guest1->Send(0, SysCh::RELIABLE, segment1);

    REQUIRE(err == Error::OK);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (2)]");

    event_status = guest1->Service(guest1_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    event_status = host->Service(host_event, 0);

    REQUIRE(event_status == EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(host_event->TypeIs(RUdpEventType::RECEIVE));
    REQUIRE(host_event->DataAsString() == msg1);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (4)]");

    event_status = guest1->Service(guest1_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    event_status = host->Service(host_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();

    LOG("");
    LOG("==================================================");
    LOG(" Step 4 : Guest 2 sends a reliable command to Host");
    LOG("==================================================");

    std::string msg2{"command from guest2"};
    auto data2 = std::vector<uint8_t>{msg2.begin(), msg2.end()};
    auto flags2 = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE);
    auto segment2 = std::make_shared<RUdpSegment>(data2, flags2);

    LOG("");
    LOG("[GUEST 2 : SEND (1)]");

    err = guest2->Send(0, SysCh::RELIABLE, segment2);

    REQUIRE(err == Error::OK);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (2)]");

    event_status = guest2->Service(guest2_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    event_status = host->Service(host_event, 0);

    REQUIRE(event_status == EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(host_event->TypeIs(RUdpEventType::RECEIVE));
    REQUIRE(host_event->DataAsString() == msg2);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (4)]");

    event_status = guest2->Service(guest2_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    event_status = host->Service(host_event, 0);

    REQUIRE(event_status == EventStatus::NO_EVENT_OCCURRED);

    DELAY_SHORT();
}

TEST_CASE("guest sends a fragmented reliable command to host", "[fragmented reliable command]")
{
    // host address
    IpAddress host_ip{"::FFFF:127.0.0.1"};
    RUdpAddress host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(10000);

    // host
    RUdpAddress address;
    address.port(10000);
    auto host = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);

    // guest1
    RUdpAddress guest1_address;
    guest1_address.port(10001);
    auto guest1 = std::make_unique<RUdpHost>(guest1_address, SysCh::MAX, 1, 100, 100);

    // guest2
    RUdpAddress guest2_address;
    guest2_address.port(10002);
    auto guest2 = std::make_unique<RUdpHost>(guest2_address, SysCh::MAX, 1, 100, 100);

    EventStatus event_status;

    auto host_event = std::make_unique<RUdpEvent>();
    auto guest1_event = std::make_unique<RUdpEvent>();
    auto guest2_event = std::make_unique<RUdpEvent>();

    LOG("==================================================");
    LOG(" Step 1 : Guest 1 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 1 : CONNECT (1)]");

    guest1->Connect(host_address, SysCh::MAX, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (2)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (4)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (6)]");

    guest1->Service(guest1_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (7)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    REQUIRE(host->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(guest1->PeerState(0) == RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Guest 2 sends CONNECT command to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST 2 : CONNECT (1)]");

    guest2->Connect(host_address, SysCh::MAX, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (2)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (3)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (4)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (5)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 2 (6)]");

    guest2->Service(guest2_event, 0);

    DELAY_SHORT();

    LOG("");
    LOG("[HOST (7)]");

    host->Service(host_event, 0);

    DELAY_SHORT();

    REQUIRE(host->PeerState(1) == RUdpPeerState::CONNECTED);
    REQUIRE(guest2->PeerState(0) == RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 3 : Guest 1 sends a fragmented segment to Host");
    LOG("==================================================");

    std::string msg1{"Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul. Out of the night that covers me, Black as the pit from pole to pole, I thank whatever gods may be For my unconquerable soul. In the fell clutch of circumstance I have not winced nor cried aloud. Under the bludgeonings of chance My head is bloody, but unbowed. Beyond this place of wrath and tears Looms but the Horror of the shade, And yet the menace of the years Finds and shall find me unafraid. It matters not how strait the gate, How charged with punishments the scroll, I am the master of my fate, I am the captain of my soul."};
    auto payload1 = std::vector<uint8_t>{msg1.begin(), msg1.end()};
    auto flags1 = static_cast<uint32_t>(RUdpSegmentFlag::RELIABLE);
    auto segment1 = std::make_shared<RUdpSegment>(payload1, flags1);

    LOG("");
    LOG("[GUEST 1 : SEND (1)]");
    REQUIRE(guest1->Send(0, SysCh::RELIABLE, segment1) == Error::OK);
    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (2)]");
    REQUIRE(guest1->Service(guest1_event, 0) == EventStatus::NO_EVENT_OCCURRED);
    DELAY_LONG();

    LOG("");
    LOG("[HOST (3)]");
    event_status = host->Service(host_event, 0);
    DELAY_LONG();

    LOG("");
    LOG("[HOST (6)]");
    event_status = host->Service(host_event, 0);
    REQUIRE(event_status == EventStatus::AN_EVENT_OCCURRED);
    REQUIRE(host_event->TypeIs(RUdpEventType::RECEIVE));
    REQUIRE(host_event->DataAsString() == msg1);
    DELAY_SHORT();

    LOG("");
    LOG("[GUEST 1 (7)]");
    REQUIRE(guest1->Service(guest1_event, 0) == EventStatus::NO_EVENT_OCCURRED);
    DELAY_SHORT();

    LOG("");
    LOG("[HOST (8)]");
    REQUIRE(host->Service(host_event, 0) == EventStatus::NO_EVENT_OCCURRED);
    DELAY_SHORT();

    LOG("");
    LOG("==================================================");
    LOG(" Step 4 : Guest 2 sends a fragmented segment to Host");
    LOG("==================================================");

    // ...
}
