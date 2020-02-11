#ifndef P2P_TECHDEMO_RUDPCOMMANDPOD_H
#define P2P_TECHDEMO_RUDPCOMMANDPOD_H

#include <array>
#include <list>
#include <map>
#include <memory>

#include "lib/rudp/peer/RUdpPeerNet.h"
#include "lib/rudp/chamber.h"
#include "lib/rudp/channel.h"
#include "command.h"

namespace rudp
{
    class CommandPod {
    public:
        CommandPod();

        bool
        LoadReliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber,
                std::unique_ptr<RUdpPeerNet> &net,
                const std::vector<std::shared_ptr<Channel>> &channels,
                uint32_t service_time);

        bool
        LoadUnreliableCommandsIntoChamber(std::unique_ptr<Chamber> &chamber, std::unique_ptr<RUdpPeerNet> &net);

        RUdpProtocolCommand
        RemoveSentReliableCommand(uint16_t reliable_sequence_number, uint8_t channel_id, std::shared_ptr<Channel> &channel);

        void
        RemoveSentUnreliableCommands();

        void
        Reset();

        void
        SetupOutgoingCommand(std::shared_ptr<RUdpOutgoingCommand> &outgoing_command, const std::shared_ptr<Channel> &channel);

        bool
        Timeout(const std::unique_ptr<RUdpPeerNet> &net, uint32_t service_time);

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
        IncrementOutgoingReliableSequenceNumber() {
            core::Singleton<core::Logger>::Instance().Debug(
                    "outgoing reliable sequence number was incremented (CommandPod): {0} -> {1}",
                    outgoing_reliable_sequence_number_,
                    outgoing_reliable_sequence_number_ + 1);
            ++outgoing_reliable_sequence_number_;
        }

        inline void
        IncrementReliableDataInTransit(uint32_t val) { reliable_data_in_transit_ += val; };

        inline
        std::map<std::string, std::list<std::shared_ptr<RUdpOutgoingCommand>>>
        Inspect() {
            std::map<std::string, std::list<std::shared_ptr<RUdpOutgoingCommand>>> ret{
                    {"sent_reliable_commands", sent_reliable_commands_},
                    {"outgoing_reliable_commands", outgoing_reliable_commands_}
            };
            return ret;
        }

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
        uint32_t round_trip_time_;          // mean round trip time (RTT), in milliseconds, between sending a reliable
        // packet and receiving its acknowledgement
        uint32_t round_trip_time_variance_;
        uint32_t timeout_limit_;
        uint32_t timeout_maximum_;
        uint32_t timeout_minimum_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCOMMANDPOD_H
