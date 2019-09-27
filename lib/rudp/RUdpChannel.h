#ifndef P2P_TECHDEMO_RUDPCHANNEL_H
#define P2P_TECHDEMO_RUDPCHANNEL_H

#include <array>
#include <list>

#include "lib/rudp/command/RUdpIncomingCommand.h"

class RUdpChannel
{
public:
    RUdpChannel();

    void
    Reset();

    inline void
    DecrementReliableWindow(size_t idx) { --reliable_windows_.at(idx); }

    inline void
    IncrementOutgoingReliableSequenceNumber() { ++outgoing_reliable_sequence_number_; }

    inline void
    IncrementOutgoingUnreliableSequenceNumber() { ++outgoing_unreliable_sequence_number_; }

    inline void
    IncrementReliableWindow(size_t idx) { ++reliable_windows_.at(idx); }

    inline void
    MarkReliableWindowAsUnused(uint16_t position) { used_reliable_windows_ &= ~ (1u << position); }

    inline void
    MarkReliableWindowAsUsed(uint16_t position) { used_reliable_windows_ |= 1u << position; }

    inline uint16_t
    ReliableWindow(size_t idx) { return reliable_windows_.at(idx); }

public:
    inline uint16_t
    incoming_reliable_sequence_number() { return incoming_reliable_sequence_number_; }

    inline uint16_t
    outgoing_reliable_sequence_number() { return outgoing_reliable_sequence_number_; }

    inline uint16_t
    outgoing_unreliable_sequence_number() { return outgoing_unreliable_sequence_number_; }

    inline void
    outgoing_unreliable_sequence_number(uint16_t val) { outgoing_unreliable_sequence_number_ = val; }

    inline uint16_t
    used_reliable_windows() { return used_reliable_windows_; }

private:
    std::list<RUdpIncomingCommand> incoming_reliable_commands_;
    std::list<RUdpIncomingCommand> incoming_unreliable_commands_;
    std::array<uint16_t, PEER_RELIABLE_WINDOWS> reliable_windows_;

    uint16_t incoming_reliable_sequence_number_;
    uint16_t incoming_unreliable_sequence_number_;
    uint16_t outgoing_reliable_sequence_number_;
    uint16_t outgoing_unreliable_sequence_number_;
    uint16_t used_reliable_windows_; // 使用中のバッファ（reliable_windows[PEER_RELIABLE_WINDOWS]）
};

#endif // P2P_TECHDEMO_RUDPCHANNEL_H
