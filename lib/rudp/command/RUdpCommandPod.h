#ifndef P2P_TECHDEMO_RUDPCOMMANDPOD_H
#define P2P_TECHDEMO_RUDPCOMMANDPOD_H

#include <list>
#include <memory>

#include "lib/rudp/peer/RUdpPeerNet.h"
#include "lib/rudp/RUdpChamber.h"
#include "lib/rudp/RUdpChannel.h"
#include "RUdpCommand.h"

class RUdpCommandPod
{


public:
    RUdpCommandPod();

    int
    CheckTimeouts(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time);

    bool
    LoadReliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, std::unique_ptr<RUdpPeerNet> &net,
                                    const std::vector<std::shared_ptr<RUdpChannel>> &channels,
                                    uint32_t service_time);

    bool
    LoadUnreliableCommandsIntoChamber(std::unique_ptr<RUdpChamber> &chamber, std::unique_ptr<RUdpPeerNet> &net);

    RUdpProtocolCommand
    RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id,
                              std::shared_ptr<RUdpChannel> &channel);

    void
    RemoveSentUnreliableCommands();

    void
    Reset();

    void
    SetupOutgoingCommand(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command,
                         const std::shared_ptr<RUdpChannel> &channel);

    inline void
    ClearOutgoingReliableCommand() { outgoing_reliable_commands_.clear(); };

    inline void
    ClearOutgoingUnreliableCommand() { outgoing_unreliable_commands_.clear(); };

    inline void
    ClearSentReliableCommand() { sent_reliable_commands_.clear(); };

    inline void
    ClearSentUnreliableCommand() { sent_unreliable_commands_.clear(); };

    inline void
    IncrementIncomingDataTotal(uint32_t val) { incoming_data_total_ += val; }

    inline void
    IncrementOutgoingDataTotal(uint32_t val) { outgoing_data_total_ += val; }

    inline void
    IncrementReliableDataInTransit(uint32_t val) { reliable_data_in_transit_ += val; };

    inline uint32_t
    NextTimeout() { return next_timeout_; };

    inline bool
    OutgoingReliableCommandExists() { return !outgoing_reliable_commands_.empty(); };

    inline bool
    OutgoingReliableCommandNotExists() { return outgoing_reliable_commands_.empty(); };

    inline bool
    OutgoingUnreliableCommandExists() { return !outgoing_unreliable_commands_.empty(); };

    inline bool
    OutgoingUnreliableCommandNotExists() { return outgoing_unreliable_commands_.empty(); };

    inline bool
    SentReliableCommandExists() { return !sent_reliable_commands_.empty(); };

    inline bool
    SentReliableCommandNotExists() { return sent_reliable_commands_.empty(); };

public:
    inline void
    earliest_timeout(uint32_t val) { earliest_timeout_ = val; }

    inline uint32_t
    outgoing_data_total() { return outgoing_data_total_; };

    inline uint32_t
    round_trip_time() { return round_trip_time_; }

    inline void
    round_trip_time(uint32_t val) { round_trip_time_ = val; }

    inline uint32_t
    round_trip_time_variance() { return round_trip_time_variance_; }

    inline void
    round_trip_time_variance(uint32_t val) { round_trip_time_variance_ = val; }

private:
    std::list<std::shared_ptr<RUdpOutgoingCommand>> outgoing_reliable_commands_;
    std::list<std::shared_ptr<RUdpOutgoingCommand>> outgoing_unreliable_commands_;
    std::list<std::shared_ptr<RUdpOutgoingCommand>> sent_reliable_commands_;
    std::list<std::shared_ptr<RUdpOutgoingCommand>> sent_unreliable_commands_;

    uint32_t incoming_data_total_;
    uint32_t outgoing_data_total_;

    uint16_t incoming_unsequenced_group_;
    uint16_t outgoing_unsequenced_group_;

    uint16_t outgoing_reliable_sequence_number_;

    uint32_t earliest_timeout_;
    uint32_t next_timeout_;
    uint32_t reliable_data_in_transit_;
    uint32_t round_trip_time_;
    uint32_t round_trip_time_variance_;
    uint32_t timeout_limit_;
    uint32_t timeout_maximum_;
    uint32_t timeout_minimum_;
};

#endif // P2P_TECHDEMO_RUDPCOMMANDPOD_H
