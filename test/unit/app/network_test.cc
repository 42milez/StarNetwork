#include <memory>

#include <catch2/catch.hpp>

#include "app/network.h"

TEST_CASE("Create peer", "[unit][network]")
{
    auto client     = std::make_unique<app::Network>();
    auto ret_client = client->CreateClient("::FFFF:127.0.0.1", 49152, 49152, 100, 100);

    REQUIRE(ret_client == Error::OK);
}

TEST_CASE("Create server", "[unit][network]")
{
    auto server     = std::make_unique<app::Network>();
    auto ret_server = server->CreateServer(49152, 32, 100, 100);

    REQUIRE(ret_server == Error::OK);
}
