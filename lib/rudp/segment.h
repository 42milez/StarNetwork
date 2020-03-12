#ifndef P2P_TECHDEMO_LIB_RUDP_SEGMENT_H_
#define P2P_TECHDEMO_LIB_RUDP_SEGMENT_H_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "lib/core/encode.h"
#include "enum.h"
#include "type.h"

namespace rudp
{
    class Segment
    {
      public:
        Segment(const std::vector<uint8_t> *data, uint32_t flags);

        Segment(const std::vector<uint8_t> *data, uint32_t flags, uint32_t buffer_size);

        void
        AddPeerIdx(uint32_t peer_idx);

        void
        AddSysMsg(SysMsg msg);

        void
        Destroy();

        inline uint32_t
        AddFlag(uint32_t flag)
        {
            return flags_ |= flag;
        }

        inline void
        AppendData(std::vector<uint8_t> &fragment)
        {
            std::copy(fragment.begin(), fragment.end(), data_.begin() + buffer_pos_);
            buffer_pos_ += fragment.size();
        }

        inline std::vector<uint8_t>
        Data()
        {
            return data_;
        }

        inline std::vector<uint8_t>
        Data(size_t offset, size_t size)
        {
            std::vector<uint8_t> ret(size);
            auto begin = data_.begin() + offset;
            auto end   = begin + size;
            std::copy(begin, end, ret.begin());
            return ret;
        }

        inline size_t
        DataLength()
        {
            return data_.size() * sizeof(uint8_t);
        }

        inline uint32_t
        First4()
        { return core::DecodeUint32(data_, 0); }

        inline uint32_t
        Second4()
        { return core::DecodeUint32(data_, 4); }

      public:
        inline uint32_t
        flags()
        {
            return flags_;
        }

      private:
        std::vector<uint8_t> data_;
        std::vector<uint8_t> user_data_;

        size_t buffer_pos_;

        std::function<void(Segment *)> free_callback_;

        uint32_t flags_;
    };

    using SegmentSP = std::shared_ptr<Segment>;
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_SEGMENT_H_
