#ifndef P2P_TECHDEMO_CORE_OS_H
#define P2P_TECHDEMO_CORE_OS_H

#include <cstdint>

class OS
{
public:
    OS();
    uint64_t GetTicksUsec() const;
    uint32_t GetTicksMsec() const;

private:
    uint64_t clock_start_;
    double clock_scale_;
};

#endif // P2P_TECHDEMO_CORE_OS_H
