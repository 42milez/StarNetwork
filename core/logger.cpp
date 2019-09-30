#include <filesystem>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logger.h"

namespace core
{
bool
Logger::Init(const std::string &logger_name)
{
    try
    {
        stdout_ = spdlog::stdout_color_mt(logger_name);
        stderr_ = spdlog::stderr_color_mt(logger_name);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        return false;
    }

    return true;
}

bool
Logger::Init(const std::string &logger_name, const std::string &path)
{
    std::filesystem::file_status status = std::filesystem::status(path);

    if (status.type() != std::filesystem::file_type::regular)
        return false;

    try
    {
        file_ = spdlog::basic_logger_mt(logger_name, path);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        return false;
    }

#ifdef DEBUG
    file_->set_level(spdlog::level::debug);
#else
    file_->set_level(spdlog::level::info);
#endif

    return true;
}

template<class... Args>
void
Logger::Debug(Args... args) { file_ ? file_->debug(args...) : stdout_->debug(args...); };

template<class... Args>
void
Logger::Info(Args... args) { file_ ? file_->info(args...) : stdout_->info(args...); };

template<class... Args>
void
Logger::Warn(Args... args) { file_ ? file_->warn(args...) : stderr_->warn(args...); };

template<class... Args>
void
Logger::Error(Args... args) { file_ ? file_->error(args...) : stderr_->error(args...); };

template<class... Args>
void
Logger::Critical(Args... args) { file_ ? file_->critical(args...) : stderr_->critical(args...); };
} // namespace core
