#include <iostream>

#include "Logger.h"

namespace engine
{
  namespace base
  {
    void Logger::init(const std::string &logger_name, const std::string &filename) {
      // TODO: ログファイルの存在確認
      // ...

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
    }
  } // namespace base
} // namespace engine
