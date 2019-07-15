#include "RUdpSegment.h"
#include "RUdpCommon.h"

uint32_t
RUdpSegment::AddFlag(uint32_t flag)
{
    flags_ |= flag;

    return flags_;
}

void
RUdpSegment::Destroy()
{
    if (free_callback_ != nullptr)
        free_callback_(this);

    // TODO: free data_
    // ...
}

size_t
RUdpSegment::Length()
{
    return data_.size();
}

uint8_t *
RUdpSegment::move_data_pointer(uint32_t val)
{
    return data_ += val;
}
