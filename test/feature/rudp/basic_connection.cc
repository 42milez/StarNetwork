#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "lib/rudp/host.h"

#include "var.h"

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
        address.port(connection::HOST_PORT);
        host_ = std::make_unique<rudp::Host>(address, rudp::SysCh::MAX, 32, 100, 100);
        core::Singleton<core::Logger>::Instance().Init("Basic Connection Test");
    }

    rudp::EventStatus Service(std::unique_ptr<rudp::Event> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

    rudp::RUdpPeerState PeerState(size_t idx)
    { return host_->PeerState(0); }

private:
    std::shared_ptr<rudp::Host> host_;
};

TEST_CASE_METHOD(HostFixture, "Connect to Host and disconnect immediately from Host", "basic_connection")
{
    rudp::NetworkConfig guest_address;
    guest_address.port(connection::GUEST1_PORT);
    auto guest = std::make_unique<rudp::Host>(guest_address, rudp::SysCh::MAX, 32, 100, 100);

    IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(connection::HOST_PORT);

    auto guest_event = std::make_unique<rudp::Event>();
    auto host_event = std::make_unique<rudp::Event>();

    LOG("==================================================");
    LOG(" Step 1 : Connect to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST (1-1)]");

    // [Queue] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    guest->Connect(host_address, rudp::SysCh::MAX, 0);

    // [Send] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (1-2)]");

    // [Receive] PROTOCOL_COMMAND_CONNECT
    // [Queue]   PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_VERIFY_CONNECT
    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST (1-3)]");

    // [Receive] PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_ACKNOWLEDGE
    guest->Service(guest_event, 0);
    DELAY();

    LOG("");
    LOG("[HOST (1-4)]");

    // [Receive] PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    // [Queue] PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    // [Send]  PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    Service(host_event, 0);

    REQUIRE(guest->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == rudp::RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Disconnect immediately from Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST (2-1)]");

    // [Queue] PROTOCOL_COMMAND_PING
    // [Queue] PROTOCOL_COMMAND_DISCONNECT
    // [Send]  PROTOCOL_COMMAND_PING
    // [Send]  PROTOCOL_COMMAND_DISCONNECT
    guest->DisconnectNow(guest_event->peer(), 0);

    DELAY();

    LOG("");
    LOG("[HOST (2-2)]");

    // [Receive] PROTOCOL_COMMAND_PING
    // [Receive] PROTOCOL_COMMAND_DISCONNECT
    // [Queue]   PROTOCOL_COMMAND_BANDWIDTH
    // [Send]    PROTOCOL_COMMAND_BANDWIDTH
    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST (2-3)]");

    // [Receive] PROTOCOL_COMMAND_BANDWIDTH
    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (2-4)]");

    Service(host_event, 0);

    REQUIRE(PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
    REQUIRE(guest->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
}

TEST_CASE_METHOD(HostFixture, "Connect to Host and disconnect gracefully from Host", "basic_connection")
{
    rudp::NetworkConfig guest_address;
    guest_address.port(connection::GUEST1_PORT);
    auto guest = std::make_unique<rudp::Host>(guest_address, rudp::SysCh::MAX, 32, 100, 100);

    IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host(host_ip.GetIPv6());
    host_address.port(connection::HOST_PORT);

    auto guest_event = std::make_unique<rudp::Event>();
    auto host_event = std::make_unique<rudp::Event>();

    LOG("");
    LOG("==================================================");
    LOG(" Step 1 : Connect to Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST (1-1)]");

    // [Queue] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // [Send]  PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    guest->Connect(host_address, rudp::SysCh::MAX, 0);
    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (1-2)]");

    // [Receive] PROTOCOL_COMMAND_CONNECT
    // [Queue]   PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_VERIFY_CONNECT
    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST (1-3)]");

    // [Receive] PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_ACKNOWLEDGE
    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (12)]");

    // [Receive] PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    Service(host_event, 0);

    REQUIRE(guest->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == rudp::RUdpPeerState::CONNECTED);

    LOG("");
    LOG("==================================================");
    LOG(" Step 2 : Disconnect immediately from Host");
    LOG("==================================================");

    LOG("");
    LOG("[GUEST (2-1)]");

    guest->DisconnectLater(guest_event->peer(), 0);
    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (2-2)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST (2-3)]");

    guest->Service(guest_event, 0);

    DELAY();

    LOG("");
    LOG("[HOST (2-4)]");

    Service(host_event, 0);

    DELAY();

    LOG("");
    LOG("[GUEST (2-5)]");

    guest->Service(guest_event, 0);

    REQUIRE(guest->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
    REQUIRE(PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
}
