#ifndef P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_
#define P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_

#include <atomic>
#include <csignal>

#include "lib/core/logger.h"

namespace core
{
    class ExitHandler
    {
      public:
        ExitHandler();

        void
        Exit();

        void
        Exit(int signum);

        bool
        Init();

        [[nodiscard]] inline bool
        ShouldExit() const
        {
            return should_exit_;
        }

      private:
        std::atomic<bool> should_exit_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_
