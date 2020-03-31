#ifndef P2P_TECHDEMO_LIB_CORE_LOGGER_H_
#define P2P_TECHDEMO_LIB_CORE_LOGGER_H_

#include <spdlog/spdlog.h>

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
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_LOGGER_H_
