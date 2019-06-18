#ifndef P2P_TECHDEMO_RUDPCOMMANDPOD_H
#define P2P_TECHDEMO_RUDPCOMMANDPOD_H

#include <list>
#include <memory>

#include "RUdpChamber.h"
#include "RUdpChannel.h"
#include "RUdpCommand.h"
#include "RUdpPeerNet.h"

class UdpCommandPod
{
private:
    std::list<std::shared_ptr<UdpOutgoingCommand>> _outgoing_reliable_commands;

    std::list<std::shared_ptr<UdpOutgoingCommand>> _outgoing_unreliable_commands;

    uint32_t _incoming_data_total;

    uint32_t _outgoing_data_total;

    uint16_t _outgoing_reliable_sequence_number;

    uint16_t _incoming_unsequenced_group;

    uint16_t _outgoing_unsequenced_group;

    uint32_t _round_trip_time;

    uint32_t _round_trip_time_variance;

    uint32_t _timeout_limit;

    uint32_t _next_timeout;

    uint32_t _reliable_data_in_transit;

    std::list<std::shared_ptr<UdpOutgoingCommand>> _sent_reliable_commands;

    std::list<std::shared_ptr<UdpOutgoingCommand>> _sent_unreliable_commands;

    uint32_t _earliest_timeout;

    uint32_t _timeout_minimum;

    uint32_t _timeout_maximum;

public:
    UdpCommandPod();

    void setup_outgoing_command(std::shared_ptr<UdpOutgoingCommand> &outgoing_command);

    void push_outgoing_reliable_command(std::shared_ptr<UdpOutgoingCommand> &command);

    bool load_reliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber,
                                             std::unique_ptr<UdpPeerNet> &net,
                                             const std::vector<std::shared_ptr<UdpChannel>> &channels,
                                             uint32_t service_time);

    bool load_unreliable_commands_into_chamber(std::unique_ptr<UdpChamber> &chamber,
                                               std::unique_ptr<UdpPeerNet> &net);

    uint32_t outgoing_data_total();

    void outgoing_data_total(uint32_t val);

    uint32_t incoming_data_total();

    void incoming_data_total(uint32_t val);

    uint32_t next_timeout();

    void next_timeout(uint32_t val);

    bool outgoing_reliable_command_exists();
    bool outgoing_unreliable_command_exists();

    void clear_outgoing_reliable_command();
    void clear_outgoing_unreliable_command();

    void reset();

    void increse_reliable_data_in_transit(uint32_t val);

    uint32_t reliable_data_in_transit();

    void reliable_data_in_transit(uint32_t val);

    void sent_reliable_command(std::shared_ptr<UdpOutgoingCommand> &command, std::unique_ptr<UdpPeerNet> &net);

    void sent_unreliable_command(std::shared_ptr<UdpOutgoingCommand> &command);

    bool sent_reliable_command_exists();

    void clear_sent_reliable_command();

    bool sent_unreliable_command_exists();

    void clear_sent_unreliable_command();

    int check_timeouts(const std::unique_ptr<UdpPeerNet> &net, uint32_t service_time);

    void remove_sent_unreliable_commands();
};

#endif // P2P_TECHDEMO_RUDPCOMMANDPOD_H
