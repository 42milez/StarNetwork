#define CATCH_CONFIG_MAIN

#include <string>

#include <catch/catch.hpp>

#include "core/io/ip_address.h"

std::string
make_ip_address(const std::string &str)
{
    auto ip_address = core::io::IpAddress(str);

    return ip_address.to_string();
}

TEST_CASE("", "")
{
    REQUIRE(make_ip_address("::FFFF:129.144.52.38") == "");
}
