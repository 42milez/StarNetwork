#include <fstream>
#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logger.h"

namespace core
{
    Logger::Logger()
        : debug_()
    {
        spdlog::stdout_color_mt("stdout");
        spdlog::stderr_color_mt("stderr");

#ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%Y/%m/%d %H:%M:%S %z][%n][%^---%L---%$][thread %t] %v");
        debug_ = true;
#else
        spdlog::set_level(spdlog::level::info);
        spdlog::set_pattern("[%Y/%m/%d %H:%M:%S %z][%n][%^---%L---%$][thread %t] %v");
#endif
    }

    // bool
    // Logger::Init(const std::string &logger_name, const std::string &path)
    //{
    //    std::ofstream{path};
    //
    //    std::filesystem::file_status status = std::filesystem::status(path);
    //
    //    if (status.type() != std::filesystem::file_type::regular)
    //        return false;
    //
    //    try
    //    {
    //        file_ = spdlog::basic_logger_mt(logger_name, path);
    //    }
    //    catch (const spdlog::spdlog_ex &ex)
    //    {
    //        return false;
    //    }
    //
    //#ifdef DEBUG
    //    file_->set_level(spdlog::level::debug);
    //#else
    //    file_->set_level(spdlog::level::info);
    //#endif
    //
    //    return true;
    //}
} // namespace core
