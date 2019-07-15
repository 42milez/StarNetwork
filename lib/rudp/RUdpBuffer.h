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
    void Push(const VecUInt8SP &data);

private:
    std::vector<const std::variant<const RUdpProtocolTypeSP, const VecUInt8SP>> data_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
