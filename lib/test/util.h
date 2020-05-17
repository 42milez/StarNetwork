#ifndef P2P_TECHDEMO_LIB_TEST_UTIL_H_
#define P2P_TECHDEMO_LIB_TEST_UTIL_H_

#include "lib/core/logger.h"
#include "lib/core/os.h"
#include "lib/core/singleton.h"

namespace test
{
    inline void
    log(const std::string &message)
    {
        core::Singleton<core::Logger>::Instance().Debug(message);
    }

    inline bool
    wait(std::function<bool()> &&f, uint16_t timeout)
    {
        auto start_at = core::Singleton<core::OS>::Instance().GetTicksMsec();

        while (true) {
            if (f()) {
                return true;
            }

            if (core::Singleton<core::OS>::Instance().GetTicksMsec() - start_at > timeout) {
                return false;
            }
        }
    }
} // namespace test

#endif // P2P_TECHDEMO_LIB_TEST_UTIL_H_
