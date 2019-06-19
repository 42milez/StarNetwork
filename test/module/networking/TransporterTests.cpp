#define CATCH_CONFIG_MAIN

#include <memory>

#include <catch2/catch.hpp>

#include "module/networking/transporter.h"

TEST_CASE("", "")
{
    std::unique_ptr<Transporter> server = std::make_unique<Transporter>();
    auto ret_server = server->create_server(8888, 32, 100, 100);

    REQUIRE(ret_server == Error::OK);

    std::unique_ptr<Transporter> client = std::make_unique<Transporter>();
    auto ret_client = client->create_client("::FFFF:127.0.0.1", 8888, 100, 100, 8889);

    REQUIRE(ret_client == Error::OK);
}
