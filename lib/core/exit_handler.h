#ifndef P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_
#define P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_

#include <atomic>
#include <csignal>

namespace core
{
    enum class REGISTER_HANDLER_STATUS
    {
        SUCCESS = 0,
        FAIL    = -1
    };

    class ExitHandler
    {
      public:
        ExitHandler();

        bool
        Init();

        inline void
        Exit()
        {
            should_exit_ = true;
            LOG_INFO("gracefully shutting down...")
        }

        inline void
        Exit(int signum)
        {
            should_exit_ = true;
            LOG_INFO_VA("gracefully shutting down... ({0})", signum)
        }

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
