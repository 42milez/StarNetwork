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

        core::Singleton<core::Logger>::Instance().Init("BasicConnection");
    }

    EventStatus Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

    RUdpPeerState PeerState(size_t idx)
    { return host_->PeerState(0); }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(ServerIPv4Fixture, "Ping (1)", "[IPv4]")
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
    Service(server_event, 0);

    REQUIRE(PeerState(0) == RUdpPeerState::CONNECTED);

    // ==================================================
    //  Step 2 : Send ping command to the peer
    // ==================================================

//    sleep(1);
//
//    Service(server_event, 0); // enqueue ping command
//    Service(server_event, 0); // send ping command
//
//    usleep(SLEEP_DURATION);
//
//    client->Service(client_event, 0);
}