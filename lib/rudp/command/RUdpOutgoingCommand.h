#ifndef P2P_TECHDEMO_RUDPOUTGOINGCOMMAND_H
#define P2P_TECHDEMO_RUDPOUTGOINGCOMMAND_H

#include "RUdpCommand.h"

class RUdpOutgoingCommand : public RUdpCommand
{
public:
    RUdpOutgoingCommand();

    inline void
    IncrementSendAttempts() { ++send_attempts_; }

public:
    inline uint16_t
    fragment_length() { return fragment_length_; }

    inline void
    fragment_length(uint16_t val) { fragment_length_ = val; }

    inline uint32_t
    fragment_offset() { return fragment_offset_; }

    inline void
    fragment_offset(uint32_t val) { fragment_offset_ = val; }

    inline uint16_t
    unreliable_sequence_number() { return unreliable_sequence_number_; }

    inline void
    unreliable_sequence_number(uint16_t val) { unreliable_sequence_number_ = val; }

    inline uint16_t
    reliable_sequence_number() { return reliable_sequence_number_; }

    inline void
    reliable_sequence_number(uint16_t val) { reliable_sequence_number_ = val; }

    inline uint32_t
    round_trip_timeout() { return round_trip_timeout_; }

    inline void
    round_trip_timeout(uint32_t val) { round_trip_timeout_ = val; }

    inline uint32_t
    round_trip_timeout_limit() { return round_trip_timeout_limit_; }

    inline void
    round_trip_timeout_limit(uint32_t val) { round_trip_timeout_limit_ = val; }

    inline uint16_t
    send_attempts() { return send_attempts_; }

    inline void
    send_attempts(uint16_t val) { send_attempts_ = val; }

    inline uint32_t
    sent_time() { return sent_time_; }

    inline void
    sent_time(uint32_t val) { sent_time_ = val; }

private:
    uint32_t fragment_offset_;
    uint16_t fragment_length_;

    uint32_t round_trip_timeout_;
    uint32_t round_trip_timeout_limit_;

    uint32_t sent_time_;
    uint16_t send_attempts_;

    uint16_t reliable_sequence_number_;
    uint16_t unreliable_sequence_number_;
};

#endif // P2P_TECHDEMO_RUDPOUTGOINGCOMMAND_H
