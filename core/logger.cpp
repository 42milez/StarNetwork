#include <filesystem>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>

#include "initialization_exception.h"
#include "logger.h"

Logger::Logger(const std::string &logger_name, const std::string &filename)
{
    if (!std::filesystem::exists(filename))
        throw InitializationException{"File does not exist: " + filename};

    try
    {
        _logger = spdlog::basic_logger_mt(logger_name, filename);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        throw InitializationException{"Logger initialization failed: " + std::string{ex.what()}};
    }

#ifdef DEBUG
    _logger->set_level(spdlog::level::debug);
#else
    _logger->set_level(spdlog::level::info);
#endif
}
