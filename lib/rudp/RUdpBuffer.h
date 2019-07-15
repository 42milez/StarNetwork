#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <memory>
#include <variant>
#include <vector>

#include <cstddef>

#include "RUdpCommon.h"

using RUdpBuffer = struct RUdpBuffer
{
public:
    void Push(const RUdpProtocolTypeSP &cmd);
    void Push(const std::shared_ptr<ReadRange> &range);

private:
    std::vector<const std::variant<const RUdpProtocolTypeSP, std::shared_ptr<ReadRange>>> data_;
};

using RUdpBufferSP = std::shared_ptr<RUdpBuffer>;

#endif // P2P_TECHDEMO_RUDPBUFFER_H
