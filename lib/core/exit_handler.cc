#include "exit_handler.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"

namespace core
{
    namespace
    {
        using SignalHandler = void (*)(int sig, siginfo_t *info, void *ctx);

        REGISTER_HANDLER_STATUS
        IgnoreSignal(int signum)
        {
            struct sigaction act
            {
            };

            act.sa_handler = SIG_IGN;
            act.sa_flags   = SA_NODEFER;

            return static_cast<REGISTER_HANDLER_STATUS>(sigaction(signum, &act, nullptr));
        }

        REGISTER_HANDLER_STATUS
        RegisterHandler(int signum, SignalHandler handler)
        {
            struct sigaction act
            {
            };

            act.sa_sigaction = handler;
            act.sa_flags     = SA_SIGINFO;

            return static_cast<REGISTER_HANDLER_STATUS>(sigaction(signum, &act, nullptr));
        }

        void
        AbnormalTerminationHandler(int signum, siginfo_t *info, void *ctx)
        {
            LOG_DEBUG_VA("[SIGABRT] si_signo:{0}, si_code:{1}, si_pid:{2}, si_uid:{3}", info->si_signo, info->si_code,
                         (int)info->si_pid, (int)info->si_uid);
            core::Singleton<core::ExitHandler>::Instance().Exit(signum);
        }

        void
        InteractiveAttentionHandler(int signum, siginfo_t *info, void *ctx)
        {
            LOG_DEBUG_VA("[SIGINT] si_signo:{0}, si_code:{1}, si_pid:{2}, si_uid:{3}", info->si_signo, info->si_code,
                         (int)info->si_pid, (int)info->si_uid);
            core::Singleton<core::ExitHandler>::Instance().Exit(signum);
        }

        void
        HungupHandler(int signum, siginfo_t *info, void *ctx)
        {
            LOG_DEBUG_VA("[SIGHUP] si_signo:{0}, si_code:{1}, si_pid:{2}, si_uid:{3}", info->si_signo, info->si_code,
                         (int)info->si_pid, (int)info->si_uid);
            core::Singleton<core::ExitHandler>::Instance().Exit(signum);
        }

        void
        TerminationHandler(int signum, siginfo_t *info, void *ctx)
        {
            LOG_DEBUG_VA("[SIGTERM] si_signo:{0}, si_code:{1}, si_pid:{2}, si_uid:{3}", info->si_signo, info->si_code,
                         (int)info->si_pid, (int)info->si_uid);
            core::Singleton<core::ExitHandler>::Instance().Exit(signum);
        }
    } // namespace

    ExitHandler::ExitHandler()
        : should_exit_()
    {
    }

    bool
    ExitHandler::Init()
    {
        if (IgnoreSignal(SIGPIPE) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (IgnoreSignal(SIGUSR1) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (IgnoreSignal(SIGUSR2) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (IgnoreSignal(SIGTTIN) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (IgnoreSignal(SIGTTOU) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (RegisterHandler(SIGABRT, AbnormalTerminationHandler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (RegisterHandler(SIGINT, InteractiveAttentionHandler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (RegisterHandler(SIGHUP, HungupHandler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        if (RegisterHandler(SIGTERM, TerminationHandler) == REGISTER_HANDLER_STATUS::FAIL) {
            return false;
        }

        return true;
    }

    void
    ExitHandler::Exit()
    {
        should_exit_ = true;
        LOG_INFO("gracefully shutting down...");
    }

    void
    ExitHandler::Exit(int signum)
    {
        should_exit_ = true;
        LOG_INFO_VA("gracefully shutting down... ({0})", signum);
    }
} // namespace core
