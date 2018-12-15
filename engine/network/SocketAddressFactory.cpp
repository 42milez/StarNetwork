#include "SocketAddressFactory.h"

#include <string>

#include <netdb.h>

#include "SocketUtil.h"

using std::string;

namespace engine
{
  namespace network
  {
    SocketAddressPtr SocketAddressFactory::create_ipv4_from_string(const string &in_string) {
      auto pos = in_string.find_last_of(':');
      string host, service;

      if (pos != string::npos) {
        host = in_string.substr(0, pos);
        service = in_string.substr(pos + 1);
      } else {
        host = in_string;
        service = "0";
      }

      addrinfo hint;
      memset(&hint, 0, sizeof(hint));

      hint.ai_family = AF_INET;

      addrinfo *result;
      int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &result);
      if (error != 0 && result != nullptr) {
        ::network::SocketUtil::report_error("SocketAddressFactory::create_ipv4");
        return nullptr;
      }

      while (!result->ai_addr && result->ai_next) {
        result = result->ai_next;
      }

      if (!result->ai_addr) {
        return nullptr;
      }

      auto ret = std::make_shared<SocketAddress>(*result->ai_addr);

      freeaddrinfo(result);

      return ret;
    }
  } // namespace network
} // namespace engine
