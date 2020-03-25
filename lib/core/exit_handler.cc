#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "exit_handler.h"

namespace core
{
    namespace
    {
        template <typename Func>
        REGISTER_HANDLER_STATUS
        register_handler(int signum, Func handler)
        {
            struct sigaction act
            {
                nullptr
            };
            act.sa_handler = handler;
            act.sa_flags   = 0;

            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, signum);

            act.sa_mask = set;

            return static_cast<REGISTER_HANDLER_STATUS>(sigaction(signum, nullptr, &act));
        }
    } // namespace

    ExitHandler::ExitHandler()
        : should_exit_()
    {
    }

    bool
    ExitHandler::Init()
    {
        // Handle Exit Signal
        auto sig_handler = [](int signum) {
            core::Singleton<core::Logger>::Instance().Info("gracefully shutting down...");
            core::Singleton<core::ExitHandler>::Instance().Exit();
        };

        if (register_handler(SIGABRT, sig_handler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (register_handler(SIGINT, sig_handler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (register_handler(SIGTERM, sig_handler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        // handle pipe signal
        if (register_handler(SIGPIPE, SIG_IGN) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        return true;
    }
} // namespace core
