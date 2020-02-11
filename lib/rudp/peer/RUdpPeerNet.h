#ifndef P2P_TECHDEMO_RUDPPEERNET_H
#define P2P_TECHDEMO_RUDPPEERNET_H

#include <cstdint>

#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/const.h"
#include "lib/rudp/enum.h"
#include "lib/rudp/macro.h"

namespace rudp
{
    std::string PeerStateAsString(RUdpPeerState state);

    class RUdpPeerNet
    {
    public:
        RUdpPeerNet();

        void CalculateSegmentLoss(uint32_t service_time);
        void Reset();
        void Setup();
        void UpdateSegmentThrottleCounter();

        inline bool
        ExceedsSegmentLossInterval(uint32_t service_time) { return UDP_TIME_DIFFERENCE(service_time, segment_loss_epoch_) >= PEER_SEGMENT_LOSS_INTERVAL; }

        inline bool
        ExceedsSegmentThrottleCounter() { return segment_throttle_counter_ > segment_throttle_; }

        inline void
        IncreaseSegmentsLost(uint32_t val) { segments_lost_ += val; }

        inline void
        IncreaseSegmentsSent(uint32_t val) { segments_sent_ += val; }

        inline bool
        StateIs(RUdpPeerState state) { return state_ == state; }

        inline bool
        StateIsNot(RUdpPeerState state) { return state_ != state; }

        inline bool
        StateIsGreaterThanOrEqual(RUdpPeerState state) { return state_ >= state; }

        inline bool
        StateIsLessThanOrEqual(RUdpPeerState state) { return state_ <= state; }

    public:
        inline void
        last_send_time(uint32_t val) { last_send_time_ = val; }

        inline uint32_t
        mtu() { return mtu_; }

        inline void
        mtu(uint32_t val) { mtu_ = val; }

        inline RUdpPeerState
        state() { return state_; }

        inline void
        state(const RUdpPeerState &val) {
            core::Singleton<core::Logger>::Instance().Debug("peer state was changed: {0} -> {1}",
                    PeerStateAsString(state_),
                    PeerStateAsString(val));
            state_ = val;
        }

        inline uint32_t
        incoming_bandwidth() { return incoming_bandwidth_; }

        inline void
        incoming_bandwidth(uint32_t val) { incoming_bandwidth_ = val; }

        inline uint32_t
        incoming_bandwidth_throttle_epoch() { return incoming_bandwidth_throttle_epoch_; }

        inline void
        incoming_bandwidth_throttle_epoch(uint32_t val) { incoming_bandwidth_throttle_epoch_ = val; }

        inline uint32_t
        outgoing_bandwidth() { return outgoing_bandwidth_; }

        inline void
        outgoing_bandwidth(uint32_t val) { outgoing_bandwidth_ = val; }

        inline uint32_t
        outgoing_bandwidth_throttle_epoch() { return outgoing_bandwidth_throttle_epoch_; }

        inline void
        outgoing_bandwidth_throttle_epoch(uint32_t val) { outgoing_bandwidth_throttle_epoch_ = val; }

        inline uint32_t
        segment_loss_epoch() { return segment_loss_epoch_; }

        inline void
        segment_loss_epoch(uint32_t val) { segment_loss_epoch_ = val; }

        inline uint32_t
        segment_throttle() { return segment_throttle_; }

        inline void
        segment_throttle(uint32_t val) { segment_throttle_ = val; }

        inline uint32_t
        segment_throttle_epoch() { return segment_throttle_epoch_; }

        inline void
        segment_throttle_epoch(uint32_t val) { segment_loss_epoch_ = val; }

        inline uint32_t
        segment_throttle_limit() { return segment_throttle_limit_; }

        inline void
        segment_throttle_limit(uint32_t val) { segment_throttle_limit_ = val; }

        inline uint32_t
        segment_throttle_interval() { return segment_throttle_interval_; }

        inline void
        segment_throttle_interval(uint32_t val) { segment_throttle_interval_ = val; }

        inline uint32_t
        segment_throttle_acceleration() { return segment_throttle_acceleration_; }

        inline void
        segment_throttle_acceleration(uint32_t val) { segment_throttle_acceleration_ = val; }

        inline uint32_t
        segment_throttle_deceleration() { return segment_throttle_deceleration_; }

        inline void
        segment_throttle_deceleration(uint32_t val) { segment_throttle_deceleration_ = val; }

        inline uint32_t
        segments_sent() { return segments_sent_; }

        inline uint32_t
        window_size() { return window_size_; }

        inline void
        window_size(uint32_t val) { window_size_ = val; }

    private:
        RUdpPeerState state_;

        uint32_t incoming_bandwidth_;
        uint32_t incoming_bandwidth_throttle_epoch_;
        uint32_t last_send_time_;
        uint32_t mtu_;
        uint32_t outgoing_bandwidth_;
        uint32_t outgoing_bandwidth_throttle_epoch_;
        uint32_t segment_throttle_;
        uint32_t segment_throttle_acceleration_;
        uint32_t segment_throttle_counter_;
        uint32_t segment_throttle_deceleration_;
        uint32_t segment_throttle_epoch_;
        uint32_t segment_throttle_interval_;
        uint32_t segment_throttle_limit_;
        uint32_t segment_loss_;
        uint32_t segment_loss_epoch_;
        uint32_t segment_loss_variance_;
        uint32_t segments_lost_;
        uint32_t segments_sent_;
        uint32_t window_size_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPPEERNET_H
