#define CATCH_CONFIG_MAIN

#include <memory>

#include <catch2/catch.hpp>

#include "module/Network.h"

TEST_CASE("Create client", "[IPv4][Network]")
{
    std::unique_ptr<Network> client = std::make_unique<Network>();
    auto ret_client = client->CreateClient("::FFFF:127.0.0.1", 8888, 8889, 100, 100);

    REQUIRE(ret_client == Error::OK);
}

TEST_CASE("Create server", "[IPv4][Network]")
{
    std::unique_ptr<Network> server = std::make_unique<Network>();
    auto ret_server = server->CreateServer(8888, 32, 100, 100);

    REQUIRE(ret_server == Error::OK);
}
