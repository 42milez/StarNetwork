#define CATCH_CONFIG_MAIN

#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/rudp/RUdpHost.h"

class ServerIPv4Fixture
{
public:
    ServerIPv4Fixture()
    {
        RUdpAddress address;
        address.port(8888);

        host_ = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);
    }

    EventStatus Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

    RUdpPeerState PeerState(size_t idx)
    { return host_->PeerState(0); }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(ServerIPv4Fixture, "Connect to the server and disconnect from the server (1)", "[IPv4]")
{
    RUdpAddress client_address;
    client_address.port(8889);
    auto client = std::make_unique<RUdpHost>(client_address, SysCh::MAX, 32, 100, 100);

    IpAddress server_ip{"::FFFF:127.0.0.1"};
    RUdpAddress server_address;
    server_address.host(server_ip.GetIPv6());
    server_address.port(8888);

    const auto SLEEP_DURATION = 100 * 1000; // millisecond

    // ==================================================
    //  Step 1 : Connect to the server
    // ==================================================

    std::unique_ptr<RUdpEvent> client_event = std::make_unique<RUdpEvent>();

    //  Queue command: PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Connect(server_address, SysCh::MAX, 0);

    //  Send command: PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Service(client_event, 0);

    usleep(SLEEP_DURATION);

    std::unique_ptr<RUdpEvent> server_event = std::make_unique<RUdpEvent>();

    //  Receive command: PROTOCOL_COMMAND_CONNECT
    //  Queue command: PROTOCOL_COMMAND_VERIFY_CONNECT
    //  Send command: PROTOCOL_COMMAND_VERIFY_CONNECT
    // --------------------------------------------------
    Service(server_event, 0);

    usleep(SLEEP_DURATION);

    //  Receive command: PROTOCOL_COMMAND_VERIFY_CONNECT
    //  Send command: PROTOCOL_COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Service(client_event, 0);

    REQUIRE(client->PeerState(0) == RUdpPeerState::CONNECTED);

    usleep(SLEEP_DURATION);

    //  Receive command: PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    // --------------------------------------------------
    auto ret = Service(server_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(ret == EventStatus::NO_EVENT_OCCURRED);

    // ==================================================
    //  Step 2 : Disconnect from the server ( use RUdpHost::DisconnectNow() )
    // ==================================================

    client->DisconnectNow(client_event->peer(), 0);

    usleep(SLEEP_DURATION);

    Service(server_event, 0);

    usleep(SLEEP_DURATION);

    client->Service(client_event, 0);

    usleep(SLEEP_DURATION);

    Service(server_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::DISCONNECTED);
    REQUIRE(client->PeerState(0) == RUdpPeerState::DISCONNECTED);
}

TEST_CASE_METHOD(ServerIPv4Fixture, "Connect to the server and disconnect from the server (2)", "[IPv4]")
{
    RUdpAddress client_address;
    client_address.port(8889);
    auto client = std::make_unique<RUdpHost>(client_address, SysCh::MAX, 32, 100, 100);

    IpAddress server_ip{"::FFFF:127.0.0.1"};
    RUdpAddress server_address;
    server_address.host(server_ip.GetIPv6());
    server_address.port(8888);

    const auto SLEEP_DURATION = 100 * 1000; // millisecond

    // ==================================================
    //  Step 1 : Connect to the server
    // ==================================================

    std::unique_ptr<RUdpEvent> client_event = std::make_unique<RUdpEvent>();

    //  Queue command: PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Connect(server_address, SysCh::MAX, 0);

    //  Send command: PROTOCOL_COMMAND_CONNECT with RUdpProtocolFlag::COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Service(client_event, 0);

    usleep(SLEEP_DURATION);

    std::unique_ptr<RUdpEvent> server_event = std::make_unique<RUdpEvent>();

    //  Receive command: PROTOCOL_COMMAND_CONNECT
    //  Queue command: PROTOCOL_COMMAND_VERIFY_CONNECT
    //  Send command: PROTOCOL_COMMAND_VERIFY_CONNECT
    // --------------------------------------------------
    Service(server_event, 0);

    usleep(SLEEP_DURATION);

    //  Receive command: PROTOCOL_COMMAND_VERIFY_CONNECT
    //  Send command: PROTOCOL_COMMAND_ACKNOWLEDGE
    // --------------------------------------------------
    client->Service(client_event, 0);

    REQUIRE(client->PeerState(0) == RUdpPeerState::CONNECTED);

    usleep(SLEEP_DURATION);

    //  Receive command: PROTOCOL_COMMAND_ACKNOWLEDGEMENT
    // --------------------------------------------------
    auto ret = Service(server_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);
    REQUIRE(ret == EventStatus::NO_EVENT_OCCURRED);

    // ==================================================
    //  Step 2 : Disconnect from the server ( use RUdpHost::DisconnectLater() )
    // ==================================================

    client->DisconnectLater(client_event->peer(), 0);
    client->Service(client_event, 0);

    usleep(SLEEP_DURATION);

    Service(server_event, 0);

    usleep(SLEEP_DURATION);

    client->Service(client_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::DISCONNECTED);
    REQUIRE(client->PeerState(0) == RUdpPeerState::DISCONNECTED);
}
