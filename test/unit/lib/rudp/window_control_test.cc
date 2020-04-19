#include <catch2/catch.hpp>

#include "lib/rudp/command/command_pod.h"
#include "lib/rudp/protocol/protocol_type.h"
#include "lib/rudp/time.h"

TEST_CASE("skip buffering reliable command when the reliable window wraps", "[unit][window_control]")
{
    auto protocol_type            = std::make_shared<rudp::ProtocolType>();
    protocol_type->header.command = static_cast<uint8_t>(rudp::RUdpProtocolCommand::SEND_RELIABLE) |
                                    static_cast<uint16_t>(rudp::RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
    protocol_type->header.channel_id = 1;
    protocol_type->send_reliable.data_length = 0;

    auto command_pod = std::make_shared<rudp::CommandPod>();

    SECTION("setting up command")
    {
        auto RELIABLE_COMMANDS = 5000;

        for (auto i = 0; i < RELIABLE_COMMANDS; ++i) {
            auto outgoing_command = std::make_shared<rudp::OutgoingCommand>();
            outgoing_command->command(protocol_type);
            outgoing_command->fragment_offset(0);
            outgoing_command->fragment_length(0);

            command_pod->SetupOutgoingCommand(outgoing_command, nullptr);
        }

        auto properties                      = command_pod->Inspect();
        auto prop_sent_reliable_commands     = properties.at("sent_reliable_commands");
        auto prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");

        REQUIRE(prop_sent_reliable_commands.empty());
        REQUIRE(prop_outgoing_reliable_commands.size() == 5000);

        SECTION("skip buffering reliable command")
        {
            auto chamber      = std::make_unique<rudp::Chamber>();
            auto net          = std::make_unique<rudp::PeerNet>();
            auto service_time = 0;
            auto channels     = {
                std::make_shared<rudp::Channel>(),
                std::make_shared<rudp::Channel>(),
                std::make_shared<rudp::Channel>(),
                std::make_shared<rudp::Channel>()
            };

            chamber->continue_sending(true);

            while (chamber->continue_sending()) {
                chamber->continue_sending(false);
                chamber->header_flags(0);
                chamber->command_count(0);
                chamber->buffer_count(1);
                chamber->segment_size(sizeof(rudp::ProtocolHeader));

                command_pod->LoadReliableCommandsIntoChamber(chamber, net, channels, service_time);
            }

            properties                      = command_pod->Inspect();
            prop_sent_reliable_commands     = properties.at("sent_reliable_commands");
            prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");

            REQUIRE(prop_sent_reliable_commands.size() == 4095);
            REQUIRE(prop_outgoing_reliable_commands.size() == 905);
        }
    }
}
