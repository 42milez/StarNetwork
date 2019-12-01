#include "core/logger.h"
#include "core/singleton.h"
#include "RUdpEnum.h"
#include "RUdpSegment.h"

namespace
{
    const uint32_t ALLOCATE_BUFFER_WITH_PAYLOAD_SIZE = 0;
}

RUdpSegment::RUdpSegment(VecUInt8 &data, uint32_t flags, uint32_t buffer_size = ALLOCATE_BUFFER_WITH_PAYLOAD_SIZE)
    : flags_(flags),
      free_callback_(),
      user_data_()
{
    if (!data.empty()) {
        try {
            auto allocation_size = 0;

            if (buffer_size == ALLOCATE_BUFFER_WITH_PAYLOAD_SIZE) {
                allocation_size = buffer_size;
            } else {
                allocation_size = data.size();
            }

            data_.resize(allocation_size);
            data_ = std::move(data);

            buffer_pos_ = data_.begin() + allocation_size;
        }
        catch (std::bad_alloc &e)
        {
            core::Singleton<core::Logger>::Instance().Critical("BAD ALLOCATION");
            throw e;
        }
    }
}

void
RUdpSegment::Destroy()
{
    if (free_callback_ != nullptr)
        free_callback_(this);

    // TODO: free buffer_
    // ...
}

void
RUdpSegment::AddSysMsg(SysMsg msg)
{
    auto msg_encoded = htonl(static_cast<uint32_t>(msg));

    memcpy(&(data_.at(0)), &msg_encoded, sizeof(uint32_t));
}

void
RUdpSegment::AddPeerIdx(uint32_t peer_idx)
{
    auto peer_idx_encoded = htonl(peer_idx);

    memcpy(&(data_.at(4)), &peer_idx_encoded, sizeof(uint32_t));
}
