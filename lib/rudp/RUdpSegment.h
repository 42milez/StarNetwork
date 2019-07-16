#ifndef P2P_TECHDEMO_RUDPSEGMENT_H
#define P2P_TECHDEMO_RUDPSEGMENT_H

#include <functional>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "RUdpCommon.h"

class RUdpSegment
{
public:
    RUdpSegment();

    uint32_t AddFlag(uint32_t flag);

    size_t Length();

    //uint8_t *move_data_pointer(uint32_t val);

    void Destroy();

    std::shared_ptr<DataRange> DataPosition(uint32_t val);

private:
    std::function<void(RUdpSegment *)> free_callback_;

    std::vector<uint8_t> data_;
    std::vector<uint8_t> user_data_;

    VecUInt8It current_read_position_;

    size_t data_length_;

    uint32_t flags_;
};

#endif // P2P_TECHDEMO_RUDPSEGMENT_H
