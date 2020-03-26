#ifndef P2P_TECHDEMO_LIB_RUDP_EVENT_H_
#define P2P_TECHDEMO_LIB_RUDP_EVENT_H_

#include "enum.h"
#include "lib/rudp/peer/peer.h"

namespace rudp
{
    class Event
    {
      public:
        Event();

        void
        Reset();

        inline core::SysCh
        Channel()
        { return static_cast<core::SysCh>(channel_id_); }

        inline std::string
        DataAsString()
        {
            return segment_->ToString();
        }

        inline uint32_t
        Id()
        {
            return segment_->ExtractByte(4);
        }

        inline core::SysMsg
        Message()
        {
            return static_cast<core::SysMsg>(segment_->ExtractByte(0));
        }

        inline size_t
        PayloadLength()
        {
            return segment_->DataLength();
        }

        inline uint32_t
        ReceiverId()
        { return segment_->ExtractByte(4); }

        inline uint32_t
        SenderId()
        { return segment_->ExtractByte(0); }

        inline bool
        TypeIs(EventType val)
        {
            return type_ == val;
        }

        inline bool
        TypeIsNot(EventType val)
        {
            return type_ != val;
        }

      public:
        inline uint8_t
        channel_id()
        {
            return channel_id_;
        }

        inline void
        channel_id(uint8_t val)
        {
            channel_id_ = val;
        }

        inline void
        data(uint32_t val)
        {
            data_ = val;
        }

        inline uint32_t
        data()
        {
            return data_;
        }

        inline std::shared_ptr<Peer>
        peer()
        {
            return peer_;
        }

        inline void
        peer(std::shared_ptr<Peer> &val)
        {
            peer_ = val;
        }

        inline std::shared_ptr<Segment>
        segment()
        { return segment_; }

        inline void
        segment(std::shared_ptr<Segment> &val)
        {
            segment_ = val;
        }

        inline void
        type(EventType val)
        {
            type_ = val;
        }

      private:
        std::shared_ptr<Peer> peer_;
        std::shared_ptr<Segment> segment_;

        EventType type_;

        uint32_t data_;

        uint8_t channel_id_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_EVENT_H_
