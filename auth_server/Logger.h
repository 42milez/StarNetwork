#ifndef P2P_TECHDEMO_AUTHSERVER_LOGGER_H
#define P2P_TECHDEMO_AUTHSERVER_LOGGER_H

#include <string>

#include "engine/base/LoggerBase.h"

namespace auth_server
{
  class Logger : public engine::base::LoggerBase {
  public:
    Logger();
  };
} // namespace auth_server

#endif // P2P_TECHDEMO_AUTHSERVER_LOGGER_H
