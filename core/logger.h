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
    Info(Args... args) { logger_->info(args...); };

    template<class... Args>
    void
    Warn(Args... args) { logger_->warn(args...); };

    template<class... Args>
    void
    Error(Args... args) { logger_->error(args...); };

    template<class... Args>
    void
    Critical(Args... args) { logger_->critical(args...); };

private:
    std::shared_ptr<spdlog::logger> logger_;
};
} // namespace core

#endif // P2P_TECHDEMO_CORE_LOGGER_H
