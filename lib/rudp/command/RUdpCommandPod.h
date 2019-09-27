#ifndef P2P_TECHDEMO_RUDPCOMMANDPOD_H
#define P2P_TECHDEMO_RUDPCOMMANDPOD_H

#include <list>
#include <memory>

#include "lib/rudp/RUdpChamber.h"
#include "lib/rudp/RUdpChannel.h"
#include "lib/rudp/RUdpPeerNet.h"
#include "RUdpCommand.h"

class RUdpCommandPod
{
private:
    std::list<std::shared_ptr<RUdpOutgoingCommand>> _outgoing_reliable_commands;

    std::list<std::shared_ptr<RUdpOutgoingCommand>> _outgoing_unreliable_commands;

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

    std::list<std::shared_ptr<RUdpOutgoingCommand>> _sent_reliable_commands;

    std::list<std::shared_ptr<RUdpOutgoingCommand>> _sent_unreliable_commands;

    uint32_t _earliest_timeout;

    uint32_t _timeout_minimum;

    uint32_t _timeout_maximum;

public:
    RUdpCommandPod();

    void setup_outgoing_command(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command,
                                const std::shared_ptr<RUdpChannel> &channel);

    void push_outgoing_reliable_command(std::shared_ptr<RUdpOutgoingCommand> &command);

    bool LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber,
                                         std::unique_ptr<RUdpPeerNet> &net,
                                         const std::vector<std::shared_ptr<RUdpChannel>> &channels,
                                         uint32_t service_time);

    bool LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber,
                                           std::unique_ptr<RUdpPeerNet> &net);

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

    void Reset();

    void increse_reliable_data_in_transit(uint32_t val);

    uint32_t reliable_data_in_transit();

    void reliable_data_in_transit(uint32_t val);

    void sent_reliable_command(std::shared_ptr<RUdpOutgoingCommand> &command, std::unique_ptr<RUdpPeerNet> &net);

    void sent_unreliable_command(std::shared_ptr<RUdpOutgoingCommand> &command);

    bool sent_reliable_command_exists();

    void clear_sent_reliable_command();

    bool sent_unreliable_command_exists();

    void clear_sent_unreliable_command();

    int check_timeouts(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time);

    RUdpProtocolCommand RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id,
                                                  std::shared_ptr<RUdpChannel> &channel);

    void remove_sent_unreliable_commands();

    inline uint32_t round_trip_time()
    { return _round_trip_time; }

    inline void round_trip_time(uint32_t val)
    { _round_trip_time = val; }

    inline uint32_t round_trip_time_variance()
    { return _round_trip_time_variance; }

    inline void round_trip_time_variance(uint32_t val)
    { _round_trip_time_variance = val; }

    inline void
    earliest_timeout(uint32_t val) { _earliest_timeout = val; }
};

#endif // P2P_TECHDEMO_RUDPCOMMANDPOD_H
