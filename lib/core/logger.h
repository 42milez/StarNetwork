#ifndef P2P_TECHDEMO_LIB_CORE_LOGGER_H_
#define P2P_TECHDEMO_LIB_CORE_LOGGER_H_

#include <spdlog/spdlog.h>

#include "lib/core/singleton.h"

namespace core
{
    class Logger
    {
      public:
        Logger();

        void
        EnableFileLogger(const std::string &log_file_path);

        template <class... Args>
        inline void
        Debug(Args... args)
        {
            if (sinks_) {
                sinks_->debug(args...);
            }
            else {
                spdlog::get("stdout")->debug(args...);
            }
        };

        template <class... Args>
        inline void
        Info(Args... args)
        {
            if (sinks_) {
                sinks_->info(args...);
            }
            else {
                spdlog::get("stdout")->info(args...);
            }
        };

        template <class... Args>
        inline void
        Warn(Args... args)
        {
            if (sinks_) {
                sinks_->warn(args...);
            }
            else {
                spdlog::get("stderr")->warn(args...);
            }
        };

        template <class... Args>
        inline void
        Error(Args... args)
        {
            if (sinks_) {
                sinks_->error(args...);
            }
            else {
                spdlog::get("stderr")->error(args...);
            }
        };

        template <class... Args>
        inline void
        Critical(Args... args)
        {
            if (sinks_) {
                sinks_->critical(args...);
            }
            else {
                spdlog::get("stderr")->critical(args...);
            }
        };

      private:
        std::shared_ptr<spdlog::logger> sinks_;
    };

#ifdef VERBOSE_LOGGING
    template <typename T>
    inline void LOG_DEBUG(T fmt)
    {
        core::Singleton<core::Logger>::Instance().Debug(fmt);
    }

    template <typename T, class... Args>
    inline void LOG_DEBUG_VA(T fmt, Args... args)
    {
        core::Singleton<core::Logger>::Instance().Debug(fmt, args...);
    }
#else
    template <typename T>
    inline void LOG_DEBUG(T fmt)
    {}

    template <typename T, class... Args>
    inline void LOG_DEBUG_VA(T fmt, Args... args)
    {}
#endif
    template <typename T>
    inline void LOG_INFO(T fmt)
    {
        core::Singleton<core::Logger>::Instance().Info(fmt);
    }

    template <typename T, class... Args>
    inline void LOG_INFO_VA(T fmt, Args... args)
    {
        core::Singleton<core::Logger>::Instance().Info(fmt, args...);
    }

    template <typename T>
    inline void LOG_CRITICAL(T fmt)
    {
        core::Singleton<core::Logger>::Instance().Critical(fmt);
    }
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_LOGGER_H_
