#include <fstream>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logger.h"

namespace core
{
    Logger::Logger()
    {
        spdlog::stdout_color_mt("stdout");
        spdlog::stderr_color_mt("stderr");
        spdlog::set_pattern("[%Y/%m/%d %H:%M:%S %z][%n][%^---%L---%$][thread %t] %v");

#ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::info);
#endif
    }
} // namespace core
