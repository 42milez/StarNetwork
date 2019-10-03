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
    {
        core::Singleton<core::Logger>::Instance().Debug(message);
    }
}

class Peer2IPv4Fixture
{
public:
    Peer2IPv4Fixture()
    {
        RUdpAddress address;
        address.port(8888);

        host_ = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);

        core::Singleton<core::Logger>::Instance().Init("BasicConnection");
    }

    EventStatus Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

    RUdpPeerState PeerState(size_t idx)
    { return host_->PeerState(0); }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(Peer2IPv4Fixture, "Connect to the peer2 and disconnect from the peer2 (1)", "[IPv4]")
{
    RUdpAddress peer1_address;
    peer1_address.port(8889);
    auto peer1 = std::make_unique<RUdpHost>(peer1_address, SysCh::MAX, 32, 100, 100);

    IpAddress peer2_ip{"::FFFF:127.0.0.1"};
    RUdpAddress peer2_address;
    peer2_address.host(peer2_ip.GetIPv6());
    peer2_address.port(8888);

    const auto SLEEP_DURATION = 100 * 1000; // millisecond

    auto peer1_event = std::make_unique<RUdpEvent>();
    auto peer2_event = std::make_unique<RUdpEvent>();

    // ==================================================
    //  Step 1 : Connect to the peer2
    // ==================================================

    LOG("[PEER 1]");

    // [Queue] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    peer1->Connect(peer2_address, SysCh::MAX, 0);

    // [Send] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    // [Receive] PROTOCOL_COMMAND_CONNECT
    // [Queue]   PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_VERIFY_CONNECT
    Service(peer2_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 1]");

    // [Receive] PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_ACKNOWLEDGE
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    // [Receive] PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    // [Queue] PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    // [Send]  PROTOCOL_COMMAND_BANDWIDTH_LIMIT
    Service(peer2_event, 0);

    REQUIRE(peer1->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);

    // ==================================================
    //  Step 2 : Disconnect from the peer2 ( use RUdpHost::DisconnectNow() )
    // ==================================================

    LOG("");
    LOG("[PEER 1]");

    // [Queue] PROTOCOL_COMMAND_PING
    // [Queue] PROTOCOL_COMMAND_DISCONNECT
    // [Send]  PROTOCOL_COMMAND_PING
    // [Send]  PROTOCOL_COMMAND_DISCONNECT
    peer1->DisconnectNow(peer1_event->peer(), 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    // [Receive] PROTOCOL_COMMAND_PING
    // [Receive] PROTOCOL_COMMAND_DISCONNECT
    // [Queue]   PROTOCOL_COMMAND_BANDWIDTH
    // [Send]    PROTOCOL_COMMAND_BANDWIDTH
    Service(peer2_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 1]");

    // [Receive] PROTOCOL_COMMAND_BANDWIDTH
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    Service(peer2_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::DISCONNECTED);
    REQUIRE(peer1->PeerState(0) == RUdpPeerState::DISCONNECTED);
}

TEST_CASE_METHOD(Peer2IPv4Fixture, "Connect to the peer2 and disconnect from the peer2 (2)", "[IPv4]")
{
    RUdpAddress peer1_address;
    peer1_address.port(8889);
    auto peer1 = std::make_unique<RUdpHost>(peer1_address, SysCh::MAX, 32, 100, 100);

    IpAddress peer2_ip{"::FFFF:127.0.0.1"};
    RUdpAddress peer2_address;
    peer2_address.host(peer2_ip.GetIPv6());
    peer2_address.port(8888);

    const auto SLEEP_DURATION = 100 * 1000; // millisecond

    auto peer1_event = std::make_unique<RUdpEvent>();
    auto peer2_event = std::make_unique<RUdpEvent>();

    // ==================================================
    //  Step 1 : Connect to the peer2
    // ==================================================

    LOG("");
    LOG("[PEER 1]");

    // [Queue] PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // [Send]  PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    peer1->Connect(peer2_address, SysCh::MAX, 0);
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    // [Receive] PROTOCOL_COMMAND_CONNECT
    // [Queue]   PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_VERIFY_CONNECT
    Service(peer2_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 1]");

    // [Receive] PROTOCOL_COMMAND_VERIFY_CONNECT
    // [Send]    PROTOCOL_COMMAND_ACKNOWLEDGE
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    // [Receive] PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    Service(peer2_event, 0);

    REQUIRE(peer1->PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);

    // ==================================================
    //  Step 2 : Disconnect from the peer2 ( use RUdpHost::DisconnectLater() )
    // ==================================================

    LOG("");
    LOG("[PEER 1]");

    peer1->DisconnectLater(peer1_event->peer(), 0);
    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    Service(peer2_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 1]");

    peer1->Service(peer1_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 2]");

    Service(peer2_event, 0);

    usleep(SLEEP_DURATION);

    LOG("");
    LOG("[PEER 1]");

    peer1->Service(peer1_event, 0);

    REQUIRE(peer1->PeerState(0) == RUdpPeerState::DISCONNECTED);
    REQUIRE(PeerState(0) == RUdpPeerState::DISCONNECTED);
}
