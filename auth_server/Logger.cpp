#include "Logger.h"

namespace auth_server
{
  namespace {
    const std::string LOGGER_NAME = "auth_server";
    const std::string PATH_LOG = "/var/log/p2p_techdemo/auth_server.log";
  }

  Logger::Logger() : LoggerBase(LOGGER_NAME, PATH_LOG) {}
} // namespace auth_server
