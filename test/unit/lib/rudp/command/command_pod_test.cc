#include <catch2/catch.hpp>

#include "lib/rudp/command/command_pod.h"
#include "lib/rudp/time.h"

TEST_CASE("command should be buffered and rebuffered on timeout", "[unit][command]")
{
    auto protocol_type            = std::make_shared<rudp::ProtocolType>();
    protocol_type->header.command = static_cast<uint8_t>(rudp::RUdpProtocolCommand::PING) |
                                    static_cast<uint16_t>(rudp::RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
    protocol_type->header.channel_id = 0xFF;

    auto outgoing_command = std::make_shared<rudp::OutgoingCommand>();
    outgoing_command->command(protocol_type);
    outgoing_command->fragment_offset(0);
    outgoing_command->fragment_length(0);

    auto command_pod = std::make_shared<rudp::CommandPod>();

    SECTION("setting up command")
    {
        command_pod->SetupOutgoingCommand(outgoing_command, nullptr);

        auto properties                      = command_pod->Inspect();
        auto prop_sent_reliable_commands     = properties.at("sent_reliable_commands");
        auto prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");

        REQUIRE(prop_sent_reliable_commands.empty());
        REQUIRE(prop_outgoing_reliable_commands.size() == 1);

        SECTION("buffering command")
        {
            auto chamber      = std::make_unique<rudp::Chamber>();
            auto channels     = {std::make_shared<rudp::Channel>()};
            auto net          = std::make_unique<rudp::PeerNet>();
            auto service_time = 0;

            command_pod->LoadReliableCommandsIntoChamber(chamber, net, channels, service_time);

            properties                      = command_pod->Inspect();
            prop_sent_reliable_commands     = properties.at("sent_reliable_commands");
            prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");

            REQUIRE(prop_sent_reliable_commands.size() == 1);
            REQUIRE(prop_outgoing_reliable_commands.empty());

            SECTION("rebuffering command on timeout")
            {
                command_pod->Timeout(net, service_time + 1000);

                properties                      = command_pod->Inspect();
                prop_sent_reliable_commands     = properties.at("sent_reliable_commands");
                prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");

                REQUIRE(prop_outgoing_reliable_commands.size() == 1);
            }
        }
    }
}
