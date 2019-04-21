#define CATCH_CONFIG_MAIN

#include <memory>

#include <catch2/catch.hpp>

#include "lib/udp/udp.h"
#include "module/networking/transporter.h"

TEST_CASE("", "")
{
    std::unique_ptr<Transporter> transporter = std::make_unique<Transporter>();
    transporter->create_server(8888, 32, 100, 100);
}
