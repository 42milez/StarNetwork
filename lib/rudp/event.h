#ifndef P2P_TECHDEMO_RUDPEVENT_H
#define P2P_TECHDEMO_RUDPEVENT_H

#include "lib/rudp/peer/RUdpPeer.h"
#include "enum.h"

namespace rudp
{
    class Event
    {
    public:
        Event();

        void
        Reset();

        inline std::string
        DataAsString() {
            auto data = segment_->Data();
            return std::string{data.begin(), data.end()};
        }

        inline bool
        TypeIs(EventType val) { return type_ == val; }

        inline bool
        TypeIsNot(EventType val) { return type_ != val; }

    public:
        inline uint8_t
        channel_id() { return channel_id_; }

        inline void
        channel_id(uint8_t val) { channel_id_ = val; }

        inline void
        data(uint32_t val) { data_ = val; }

        inline std::shared_ptr<RUdpPeer>
        peer() { return peer_; }

        inline void
        peer(std::shared_ptr<RUdpPeer> &val) { peer_ = val; }

        inline void
        segment(std::shared_ptr<RUdpSegment> &val) { segment_ = val; }

        inline void
        type(EventType val) { type_ = val; }

    private:
        std::shared_ptr<RUdpPeer> peer_;
        std::shared_ptr<RUdpSegment> segment_;

        EventType type_;

        uint32_t data_;

        uint8_t channel_id_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPEVENT_H
