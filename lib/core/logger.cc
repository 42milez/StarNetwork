#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logger.h"

namespace core
{
    Logger::Logger()
        : file_logger_()
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

    void
    Logger::EnableFileLogger(const std::string &log_file_path)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::warn);
        console_sink->set_pattern("[%Y/%m/%d %H:%M:%S %z][%^---%L---%$][thread %t] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path);
        file_sink->set_level(spdlog::level::trace);

        this->file_logger_ = std::make_unique<spdlog::logger>(spdlog::logger("p2p_techdemo", {console_sink, file_sink}));

#ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::info);
#endif
    }
} // namespace core
