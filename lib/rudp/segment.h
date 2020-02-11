#ifndef P2P_TECHDEMO_RUDPSEGMENT_H
#define P2P_TECHDEMO_RUDPSEGMENT_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "enum.h"
#include "RUdpType.h"

namespace rudp
{
    class Segment
    {
    public:
        Segment(VecUInt8 &data, uint32_t flags);

        Segment(VecUInt8 &data, uint32_t flags, uint32_t buffer_size);

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
            std::copy(fragment.begin(), fragment.end(), data_.begin() + buffer_pos_);
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

        size_t buffer_pos_;

        std::function<void(Segment *)> free_callback_;

        uint32_t flags_;
    };

    using SegmentSP = std::shared_ptr<Segment>;
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPSEGMENT_H
