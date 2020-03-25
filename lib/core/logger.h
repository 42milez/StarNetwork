#ifndef P2P_TECHDEMO_LIB_CORE_LOGGER_H_
#define P2P_TECHDEMO_LIB_CORE_LOGGER_H_

#include <spdlog/spdlog.h>

namespace core
{
    class Logger
    {
      public:
        Logger();

        template <class... Args>
        inline void
        Debug(Args... args)
        {
            spdlog::get("stdout")->debug(args...);
        };

        template <class... Args>
        inline void
        Info(Args... args)
        {
            spdlog::get("stdout")->info(args...);
        };

        template <class... Args>
        inline void
        Warn(Args... args)
        {
            spdlog::get("stderr")->warn(args...);
        };

        template <class... Args>
        inline void
        Error(Args... args)
        {
            spdlog::get("stderr")->error(args...);
        };

        template <class... Args>
        inline void
        Critical(Args... args)
        {
            spdlog::get("stderr")->critical(args...);
        };
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_LOGGER_H_
