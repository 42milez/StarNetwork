#ifndef P2P_TECHDEMO_CORE_LOGGER_H
#define P2P_TECHDEMO_CORE_LOGGER_H

#include <spdlog/spdlog.h>

namespace core
{
class Logger
{
public:
    bool
    Init(const std::string &logger_name);

    bool
    Init(const std::string &logger_name, const std::string &path);

    template<class... Args>
    void
    Debug(Args... args) { file_ ? file_->debug(args...) : stdout_->debug(args...); };

    template<class... Args>
    void
    Info(Args... args) { file_ ? file_->info(args...) : stdout_->info(args...); };

    template<class... Args>
    void
    Warn(Args... args) { file_ ? file_->warn(args...) : stderr_->warn(args...); };

    template<class... Args>
    void
    Error(Args... args) { file_ ? file_->error(args...) : stderr_->error(args...); };

    template<class... Args>
    void
    Critical(Args... args) { file_ ? file_->critical(args...) : stderr_->critical(args...); };

private:
    std::shared_ptr<spdlog::logger> stdout_;
    std::shared_ptr<spdlog::logger> stderr_;
    std::shared_ptr<spdlog::logger> file_;
};
} // namespace core

#endif // P2P_TECHDEMO_CORE_LOGGER_H
