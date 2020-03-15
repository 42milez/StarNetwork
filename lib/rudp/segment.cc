#include <cstring>

#include <arpa/inet.h>

#include "lib/core/logger.h"
#include "lib/core/network/system.h"
#include "lib/core/singleton.h"
#include "enum.h"
#include "segment.h"

namespace rudp
{
    Segment::Segment(const std::vector<uint8_t> *data, uint32_t flags)
        : buffer_pos_()
        , flags_(flags)
        , free_callback_()
        , user_data_()
    {
        if (data && !data->empty()) {
            try {
                data_.resize(data->size());
                std::copy(data->begin(), data->end(), data_.begin());
                buffer_pos_ = data->size();
            }
            catch (std::bad_alloc &e) {
                core::Singleton<core::Logger>::Instance().Critical("BAD ALLOCATION");
                throw e;
            }
        }
    }

    Segment::Segment(const std::vector<uint8_t> *data, uint32_t flags, uint32_t buffer_size)
        : buffer_pos_()
        , flags_(flags)
        , free_callback_()
        , user_data_()
    {
        if (data && !data->empty()) {
            try {
                data_.resize(buffer_size);
                std::copy(data->begin(), data->end(), data_.begin() + buffer_pos_);
                buffer_pos_ = data->size();
            }
            catch (std::bad_alloc &e) {
                core::Singleton<core::Logger>::Instance().Critical("BAD ALLOCATION");
                throw e;
            }
        }
    }

    void
    Segment::Destroy()
    {
        if (free_callback_ != nullptr)
            free_callback_(this);

        // TODO: free buffer_
        // ...
    }

    void
    Segment::AddSysMsg(core::SysMsg msg)
    {
        auto msg_encoded = htonl(static_cast<uint32_t>(msg));

        memcpy(&(data_.at(0)), &msg_encoded, sizeof(uint32_t));
    }

    void
    Segment::AddPeerIdx(uint32_t peer_idx)
    {
        auto peer_idx_encoded = htonl(peer_idx);

        memcpy(&(data_.at(4)), &peer_idx_encoded, sizeof(uint32_t));
    }
} // namespace rudp
