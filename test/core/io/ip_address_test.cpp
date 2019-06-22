#define CATCH_CONFIG_MAIN

#include <string>

#include <catch2/catch.hpp>

#include "core/io/ip_address.h"

TEST_CASE("Create IPv6 address (1)", "[IPv6][IpAddress]")
{
    std::string ip1{"::ffff:129.144.52.38"};
    std::string ip2{IpAddress{ip1}};

    REQUIRE(ip1 == ip2);
}

TEST_CASE("Create IPv6 address (2)", "[IPv6][IpAddress]")
{
    std::string ip1{"::FFFF:129.144.52.38"};
    std::string ip2{IpAddress{ip1}};

    REQUIRE(ip1 == ip2);
}

TEST_CASE("Create IPv6 address (3)", "[IPv6][IpAddress]")
{
    std::string ip1{"0:0:0:0:0:ffff:129.144.52.38"};
    std::string ip2{IpAddress{ip1}};

    REQUIRE(ip1 == ip2);
}

TEST_CASE("Create IPv6 address (4)", "[IPv6][IpAddress]")
{
    std::string ip1{"0:0:0:0:0:FFFF:129.144.52.38"};
    std::string ip2{IpAddress{ip1}};

    REQUIRE(ip1 == ip2);
}

TEST_CASE("Set and Get IPv4 address", "")
{
    IpAddress ip_address;

    uint8_t ipv4[4] = {129, 144, 52, 38};
    ip_address.set_ipv4(ipv4);

    auto ret = ip_address.get_ipv4();

    REQUIRE(ret[0] == 129);
    REQUIRE(ret[1] == 144);
    REQUIRE(ret[2] == 52);
    REQUIRE(ret[3] == 38);
}

TEST_CASE("Set and Get IPv6 address", "")
{
    IpAddress ip_address;

    uint8_t ipv6[16] = {
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF
    };
    ip_address.set_ipv6(ipv6);

    auto ret = ip_address.get_ipv6();

    REQUIRE(ret[0] == 255);
    REQUIRE(ret[1] == 255);
    REQUIRE(ret[2] == 255);
    REQUIRE(ret[3] == 255);
    REQUIRE(ret[4] == 255);
    REQUIRE(ret[5] == 255);
    REQUIRE(ret[6] == 255);
    REQUIRE(ret[7] == 255);
    REQUIRE(ret[8] == 255);
    REQUIRE(ret[9] == 255);
    REQUIRE(ret[10] == 255);
    REQUIRE(ret[11] == 255);
    REQUIRE(ret[12] == 255);
    REQUIRE(ret[13] == 255);
    REQUIRE(ret[14] == 255);
    REQUIRE(ret[15] == 255);
}
