#ifndef P2P_TECHDEMO_RUDPSEGMENT_H
#define P2P_TECHDEMO_RUDPSEGMENT_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "RUdpEnum.h"
#include "RUdpType.h"

class RUdpSegment
{
public:
    RUdpSegment(const VecUInt8SharedPtr &data, uint32_t flags);

    void
    AddPeerIdx(uint32_t peer_idx);

    void
    AddSysMsg(SysMsg msg);

    void
    Destroy();

    inline uint32_t
    AddFlag(uint32_t flag) { return flags_ |= flag; }

    inline VecUInt8SharedPtr
    Data() { return data_; }

    inline size_t
    DataLength() { return data_->size() * sizeof(uint8_t); }

public:
    inline uint32_t
    flags() { return flags_; }

private:
    VecUInt8SharedPtr data_;
    VecUInt8 user_data_;

    std::function<void(RUdpSegment *)> free_callback_;

    uint32_t flags_;
};

using RUdpSegmentSP = std::shared_ptr<RUdpSegment>;

#endif // P2P_TECHDEMO_RUDPSEGMENT_H
