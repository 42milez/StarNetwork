#include "RUdpSegment.h"
#include "RUdpSegmentFlag.h"

RUdpSegment::RUdpSegment(const VecUInt8SP &data, uint32_t flags)
    : flags_(flags),
      free_callback_(),
      user_data_()
{
    if (flags & static_cast<uint32_t>(RUdpSegmentFlag::NO_ALLOCATE))
    {
        data_ = data;
    }
    else if (data->empty())
    {
        data_ = nullptr;
    }
    else
    {
        data_ = std::make_shared<std::vector<uint8_t>>();

        if (data_ == nullptr)
        {
            // throw exception
            // ...
        }

        if (data_ != nullptr)
            std::copy(data->begin(), data->end(), data_->begin());
    }
}

void
RUdpSegment::Destroy()
{
    if (free_callback_ != nullptr)
        free_callback_(this);

    // TODO: free data_
    // ...
}

void
RUdpSegment::AddSysMsg(SysMsg msg)
{
    auto msg_encoded = htonl(static_cast<uint32_t>(msg));

    memcpy(&(data_->at(0)), &msg_encoded, sizeof(uint32_t));
}

void
RUdpSegment::AddPeerIdx(uint32_t peer_idx)
{
    auto peer_idx_encoded = htonl(peer_idx);

    memcpy(&(data_->at(4)), &peer_idx_encoded, sizeof(uint32_t));
}
