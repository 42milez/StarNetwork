#include <string>

#include <catch2/catch.hpp>

#include "lib/core/io/ip_address.h"

TEST_CASE("IpAddress can be initialized with IPv4 address", "[unit][ip_address]")
{
    std::string ipv4{"129.144.52.38"};

    SECTION("initializing with IPv4 address")
    {
        REQUIRE(ipv4 == std::string{IpAddress{"129.144.52.38"}});
    }

    SECTION("initializing with IPv4-mapped address in short representation with small letters")
    {
        REQUIRE(ipv4 == std::string{IpAddress{"::ffff:129.144.52.38"}});
    }

    SECTION("initializing with IPv4-mapped address in long representation with small letters")
    {
        REQUIRE(ipv4 == std::string{IpAddress{"0:0:0:0:0:ffff:129.144.52.38"}});
    }

    SECTION("initializing with IPv4-mapped address in short representation with capital letters")
    {
        REQUIRE(ipv4 == std::string{IpAddress{"::FFFF:129.144.52.38"}});
    }

    SECTION("initializing with IPv4-mapped address in long representation with capital letters")
    {
        REQUIRE(ipv4 == std::string{IpAddress{"0:0:0:0:0:FFFF:129.144.52.38"}});
    }
}

TEST_CASE("IpAddress can be set IPv4 address", "[unit][ip_address]")
{
    IpAddress ip_address;

    ip_address.set_ipv4({129, 144, 52, 38});
    auto ret = ip_address.GetIPv4();

    REQUIRE((ret[0] == 129 && ret[1] == 144 && ret[2] == 52 && ret[3] == 38));
}

TEST_CASE("IpAddress can be set IPv6 address", "[unit][ip_address]")
{
    IpAddress ip_address;

    ip_address.set_ipv6(
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    auto ret = ip_address.GetIPv6();

    REQUIRE((ret[0] == 255 && ret[1] == 255 && ret[2] == 255 && ret[3] == 255 && ret[4] == 255 && ret[5] == 255 &&
             ret[6] == 255 && ret[7] == 255 && ret[8] == 255 && ret[9] == 255 && ret[10] == 255 && ret[11] == 255 &&
             ret[12] == 255 && ret[13] == 255 && ret[14] == 255 && ret[15] == 255));
}
