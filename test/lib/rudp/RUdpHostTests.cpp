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

        address.port = 8888;

        host_ = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);
    }

    EventStatus Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    {
        return host_->Service(event, timeout);
    }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(ServerIPv4Fixture, "ConnectToServer", "[IPv4][RUdpHost]")
{
    RUdpAddress client_address;
    client_address.port = 8889;
    auto client = std::make_unique<RUdpHost>(client_address, SysCh::MAX, 32, 100, 100);

    IpAddress server_ip{"::FFFF:127.0.0.1"};
    RUdpAddress server_address;
    memcpy(server_address.host, server_ip.GetIPv6(), sizeof(server_address.host));
    server_address.port = 8888;

    client->Connect(server_address, SysCh::MAX, 0);
    std::unique_ptr<RUdpEvent> client_event = std::make_unique<RUdpEvent>();
    client->Service(client_event, 0);

    sleep(1);

    std::unique_ptr<RUdpEvent> server_event = std::make_unique<RUdpEvent>();
    auto ret = Service(server_event, 0); // Receive: PROTOCOL_COMMAND_CONNECT
    ret = Service(server_event, 0);      // Send: PROTOCOL_COMMAND_VERIFY_CONNECT

    sleep(1);

    client->Service(client_event, 0); // Receive: PROTOCOL_COMMAND_VERIFY_CONNECT
    client->Service(client_event, 0);

    sleep(1);

    ret = Service(server_event, 0);

    REQUIRE(ret == EventStatus::NO_EVENT_OCCURRED);
}
