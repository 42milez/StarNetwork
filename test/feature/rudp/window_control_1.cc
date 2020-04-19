#include <catch2/catch.hpp>

#include "lib/core/network/system.h"
#include "lib/rudp/host.h"
#include "lib/test/util.h"

#include "var.h"

TEST_CASE("", "[feature][reliable_command][window_control]")
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
    auto guest1       = std::make_unique<rudp::Host>(guest1_address, core::SysCh::MAX, 2, 100, 100);
    auto guest1_event = std::make_unique<rudp::Event>();

    // guest2
    rudp::NetworkConfig guest2_address;
    guest2_address.port(test::GUEST2_PORT);
    auto guest2       = std::make_unique<rudp::Host>(guest2_address, core::SysCh::MAX, 2, 100, 100);
    auto guest2_event = std::make_unique<rudp::Event>();

    SECTION("")
    {
        //  guest peer 1 [1/2]
        // --------------------------------------------------

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

        auto RELIABLE_COMMANDS = 5000;

        for (auto i = 0; i < RELIABLE_COMMANDS; ++i) {
            std::string msg{std::to_string(i)};
            auto data    = std::vector<uint8_t>{};
            auto flags   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
            auto segment = std::make_shared<rudp::Segment>(&data, flags);
            guest1->Send(0, core::SysCh::RELIABLE, segment);
            guest1->Service(guest1_event, 0);

            test::wait(
                [&host, &host_event]() {
                  return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED;
                },
                test::DEFAULT_TIMEOUT);
        }
    }
}
