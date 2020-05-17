#include "time.h"

namespace rudp
{
    uint32_t Time::time_base_ = 0;

    uint32_t
    Time::Get()
    {
        return core::Singleton<core::OS>::Instance().GetTicksMsec() - time_base_;
    }

    void
    Time::Set(uint32_t new_time_base)
    {
        time_base_ = core::Singleton<core::OS>::Instance().GetTicksMsec() - new_time_base;
    }
} // namespace rudp
