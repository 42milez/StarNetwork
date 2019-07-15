#include "RUdpSegment.h"
#include "RUdpCommon.h"

RUdpSegment::RUdpSegment() :
    current_read_position_(data_.begin()),
    data_length_(),
    flags_()
{}

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

//uint8_t *
//RUdpSegment::move_data_pointer(uint32_t val)
//{
//    return data_ += val;
//}

std::shared_ptr<ReadRange>
RUdpSegment::ReadPosition(uint32_t val)
{
    return std::make_shared<ReadRange>(current_read_position_ + val, data_.end());
}
