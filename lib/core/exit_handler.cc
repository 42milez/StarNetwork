#include "exit_handler.h"
#include "lib/core/singleton.h"

namespace core
{
    namespace
    {
        using s_exit_handler = core::Singleton<core::ExitHandler>;
    }

    ExitHandler::ExitHandler()
        : should_exit_(false)
    {
    }

    bool
    ExitHandler::init()
    {
        // Handle Exit Signal
        auto sig_handler = [](int signum) { s_exit_handler::Instance().exit(); };

        if (register_handler(SIGABRT, sig_handler) == REGISTER_HANDLER_STATUS::FAIL)
            return false;
        if (register_handler(SIGINT, sig_handler) == REGISTER_HANDLER_STATUS::FAIL)
            return false;
        if (register_handler(SIGTERM, sig_handler) == REGISTER_HANDLER_STATUS::FAIL)
            return false;
        if (register_handler(SIGPIPE, SIG_IGN) == REGISTER_HANDLER_STATUS::FAIL)
            return false; // Handle Pipe Signal

        return true;
    }

    void
    ExitHandler::exit()
    {
        should_exit_ = true;
    }

    template <typename Func>
    REGISTER_HANDLER_STATUS
    ExitHandler::register_handler(int signum, Func handler)
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

    bool
    ExitHandler::should_exit() const
    {
        return should_exit_;
    }
} // namespace core
