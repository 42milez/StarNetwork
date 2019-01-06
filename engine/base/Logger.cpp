#include <filesystem>
#include <iostream>

#include "Logger.h"

namespace engine
{
  namespace base
  {
    namespace fs = std::filesystem;

    bool Logger::init(const std::string &logger_name, const std::string &filename) {
      // TODO: check existence of the log file

      /*
       * Undefined symbols for architecture x86_64:
       * "std::__1::__fs::filesystem::__file_size(std::__1::__fs::filesystem::path const&, std::__1::error_code*)", referenced from:
       * engine::base::Logger::init(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&) in libbase.a(Logger.cpp.o)
       * ld: symbol(s) not found for architecture x86_64
       * */

      /*
       * if (!fs::file_size(filename)) {
       *   return false;
       * }
       * */

      // ロガーインスタンス生成
      logger_ = spdlog::basic_logger_mt(logger_name, filename);

      try {
        logger_ = spdlog::basic_logger_mt(logger_name, filename);
      }
      catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
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
