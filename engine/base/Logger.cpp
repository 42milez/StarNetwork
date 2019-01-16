#include <filesystem>
#include <iostream>

#include "spdlog/sinks/basic_file_sink.h"

#include "Logger.h"

namespace engine
{
  namespace base
  {
    namespace fs = std::filesystem;

    bool Logger::init(const std::string &logger_name, const std::string &filename) {
      if (!fs::exists(filename)) {
        return false;
      }

      try {
        logger_ = spdlog::basic_logger_mt(logger_name, filename);
      }
      catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return false;
      }

#ifdef DEBUG
      logger_->set_level(spdlog::level::debug);
#else
      logger_->set_level(spdlog::level::info);
#endif

      return true;
    }
  } // namespace base
} // namespace engine
