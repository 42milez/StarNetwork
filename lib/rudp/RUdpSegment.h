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
    RUdpSegment(const VecUInt8SP &data, uint32_t flags);

    uint32_t AddFlag(uint32_t flag);

    size_t Size();

    //uint8_t *move_data_pointer(uint32_t val);

    void Destroy();

    //std::shared_ptr<DataRange> DataPosition(uint32_t val);

    VecUInt8SP Data()
    { return data_; }

    void AddSysMsg(SysMsg msg);

    void AddPeerIdx(uint32_t peer_idx);

private:
    std::function<void(RUdpSegment *)> free_callback_;

    VecUInt8SP data_;
    std::vector<uint8_t> user_data_;

    //VecUInt8It current_read_position_;

    //size_t data_length_;

    uint32_t flags_;
};

#endif // P2P_TECHDEMO_RUDPSEGMENT_H
