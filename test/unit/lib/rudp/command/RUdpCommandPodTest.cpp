#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include "lib/rudp/command/RUdpCommandPod.h"
#include "lib/rudp/RUdpTime.h"

TEST_CASE("Rebuffering command after command timeout", "[timeout]")
{
  auto protocol_type = std::make_shared<RUdpProtocolType>();
  protocol_type->header.command = static_cast<uint8_t>(RUdpProtocolCommand::PING) |
                                  static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
  protocol_type->header.channel_id = 0xFF;

  auto outgoing_command = std::make_shared<RUdpOutgoingCommand>();
  outgoing_command->command(protocol_type);
  outgoing_command->fragment_offset(0);
  outgoing_command->fragment_length(0);

  std::vector<std::shared_ptr<RUdpChannel>> channels_;

  auto command_pod = std::make_shared<RUdpCommandPod>();
  command_pod->SetupOutgoingCommand(outgoing_command, nullptr);

  auto properties = command_pod->Inspect();
  auto prop_sent_reliable_commands = properties.at("sent_reliable_commands");
  auto prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");
  REQUIRE(prop_sent_reliable_commands.size() == 0);
  REQUIRE(prop_outgoing_reliable_commands.size() == 1);

  auto chamber = std::make_unique<RUdpChamber>();
  auto channels = {
      std::make_shared<RUdpChannel>(),
      std::make_shared<RUdpChannel>(),
      std::make_shared<RUdpChannel>(),
      std::make_shared<RUdpChannel>()
  };
  auto net = std::make_unique<RUdpPeerNet>();
  auto service_time = RUdpTime::Get();

  command_pod->LoadReliableCommandsIntoChamber(chamber, net, channels, service_time);

  properties = command_pod->Inspect();
  prop_sent_reliable_commands = properties.at("sent_reliable_commands");
  prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");
  REQUIRE(prop_sent_reliable_commands.size() == 1);
  REQUIRE(prop_outgoing_reliable_commands.size() == 0);

  command_pod->Timeout(net, service_time + 1000);

  properties = command_pod->Inspect();
  prop_sent_reliable_commands = properties.at("sent_reliable_commands");
  prop_outgoing_reliable_commands = properties.at("outgoing_reliable_commands");
  REQUIRE(prop_sent_reliable_commands.size() == 1);
  REQUIRE(prop_outgoing_reliable_commands.size() == 1);
}
