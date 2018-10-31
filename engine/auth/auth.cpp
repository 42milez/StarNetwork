#include "auth.h"

namespace auth
{
  Auth::Auth() {
    soc_ = std::make_shared<network::TcpSocket>();
  }

  void Auth::authorize(const std::string &id, const std::string &pw) {

  }
}
