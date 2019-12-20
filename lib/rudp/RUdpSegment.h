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
    RUdpSegment(VecUInt8 &data, uint32_t flags);

    RUdpSegment(VecUInt8 &data, uint32_t flags, uint32_t buffer_size);

    void
    AddPeerIdx(uint32_t peer_idx);

    void
    AddSysMsg(SysMsg msg);

    void
    Destroy();

    inline uint32_t
    AddFlag(uint32_t flag) { return flags_ |= flag; }

    inline void
    AppendData(std::vector<uint8_t> &fragment)
    {
        std::copy(fragment.begin(), fragment.end(), buffer_pos_);
        buffer_pos_ += fragment.size();
    }

    inline VecUInt8
    Data() { return data_; }

    inline VecUInt8
    Data(size_t offset, size_t size) {
      VecUInt8 ret(size);
      auto begin = data_.begin() + offset;
      auto end = begin + size;
      std::copy(begin, end, ret.begin());
      return ret;
    }

    inline size_t
    DataLength() { return data_.size() * sizeof(uint8_t); }

public:
    inline uint32_t
    flags() { return flags_; }

private:
    VecUInt8 data_;
    VecUInt8 user_data_;

    VecUInt8It buffer_pos_;

    std::function<void(RUdpSegment *)> free_callback_;

    uint32_t flags_;
};

using RUdpSegmentSP = std::shared_ptr<RUdpSegment>;

#endif // P2P_TECHDEMO_RUDPSEGMENT_H
