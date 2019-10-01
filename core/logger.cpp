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
        stdout_ = spdlog::stdout_color_mt(logger_name + "1");
        stderr_ = spdlog::stderr_color_mt(logger_name + "2");
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
} // namespace core
