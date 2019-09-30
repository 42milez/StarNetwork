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
    Debug(Args... args);

    template<class... Args>
    void
    Info(Args... args);

    template<class... Args>
    void
    Warn(Args... args);

    template<class... Args>
    void
    Error(Args... args);

    template<class... Args>
    void
    Critical(Args... args);

private:
    std::shared_ptr<spdlog::logger> stdout_;
    std::shared_ptr<spdlog::logger> stderr_;
    std::shared_ptr<spdlog::logger> file_;
};
} // namespace core

#endif // P2P_TECHDEMO_CORE_LOGGER_H
