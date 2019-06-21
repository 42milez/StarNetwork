#define CATCH_CONFIG_MAIN

#include <memory>

#include <catch2/catch.hpp>

#include "module/Network.h"

TEST_CASE("", "")
{
    std::unique_ptr<Network> server = std::make_unique<Network>();
    auto ret_server = server->create_server(8888, 32, 100, 100);

    REQUIRE(ret_server == Error::OK);

    std::unique_ptr<Network> client = std::make_unique<Network>();
    auto ret_client = client->create_client("::FFFF:127.0.0.1", 8888, 100, 100, 8889);

    REQUIRE(ret_client == Error::OK);
}
