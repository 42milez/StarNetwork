#define CATCH_CONFIG_MAIN

#include <memory>

#include <catch2/catch.hpp>

#include "lib/rudp/RUdpHost.h"

TEST_CASE("Service", "[IPv4][RUdpHost]")
{
    std::unique_ptr<RUdpHost> host = std::make_unique<RUdpHost>();
    std::unique_ptr<RUdpEvent> event = std::make_unique<RUdpEvent>();

    auto ret = host->Service(event, 0);

    REQUIRE(ret == EventStatus::NO_EVENT_OCCURRED);
}
