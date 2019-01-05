#ifndef P2P_TECHDEMO_LOGGERBASE_H
#define P2P_TECHDEMO_LOGGERBASE_H

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace engine
{
  namespace base
  {
    class LoggerBase {
    public:
      template<class... Args>
      void info(Args... args) { logger_->info(args...); };

      template<class... Args>
      void warn(Args... args) { logger_->warn(args...); };

      template<class... Args>
      void error(Args... args) { logger_->error(args...); };

      template<class... Args>
      void critical(Args... args) { logger_->critical(args...); };

    protected:
      LoggerBase(const std::string &logger_name, const std::string &filename);

    private:
      std::shared_ptr<spdlog::logger> logger_;
    };
  } // namespace base
} // namespace engine

#endif // P2P_TECHDEMO_LOGGERBASE_H
