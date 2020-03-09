#ifndef P2P_TECHDEMO_LIB_CORE_LOGGER_H_
#define P2P_TECHDEMO_LIB_CORE_LOGGER_H_

#include <spdlog/spdlog.h>

namespace core
{
class Logger
{
public:
    Logger();

    bool
    Init(const std::string &logger_name);

//    bool
//    Init(const std::string &logger_name, const std::string &path);

    template<class... Args>
    inline void
    Debug(Args... args) { if (debug_) file_ ? file_->debug(args...) : spdlog::get("stdout")->debug(args...); };

    template<class... Args>
    inline void
    Info(Args... args) { file_ ? file_->info(args...) : spdlog::get("stdout")->info(args...); };

    template<class... Args>
    inline void
    Warn(Args... args) { file_ ? file_->warn(args...) : spdlog::get("stderr")->warn(args...); };

    template<class... Args>
    inline void
    Error(Args... args) { file_ ? file_->error(args...) : spdlog::get("stderr")->error(args...); };

    template<class... Args>
    inline void
    Critical(Args... args) { file_ ? file_->critical(args...) : spdlog::get("stderr")->critical(args...); };

private:
    std::shared_ptr<spdlog::logger> file_;

    bool debug_;
};
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_LOGGER_H_
