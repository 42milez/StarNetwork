#include <filesystem>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>

#include "logger.h"

namespace core
{
bool
Logger::Init(const std::string &logger_name, const std::string &path)
{
    std::filesystem::file_status status = std::filesystem::status(path);

    if (status.type() != std::filesystem::file_type::regular)
        return false;

    try
    {
        logger_ = spdlog::basic_logger_mt(logger_name, path);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        return false;
    }

#ifdef DEBUG
    logger_->set_level(spdlog::level::debug);
#else
    logger_->set_level(spdlog::level::info);
#endif

    return true;
}
} // namespace core
