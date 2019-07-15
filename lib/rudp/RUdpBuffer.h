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
    void Push(const std::shared_ptr<RUdpProtocolType> &cmd);
    void Push(const std::vector<uint8_t> &data);

private:
    std::vector<const std::variant<std::shared_ptr<RUdpProtocolType>, const std::vector<uint8_t>&>> data_;
};

#endif // P2P_TECHDEMO_RUDPBUFFER_H
