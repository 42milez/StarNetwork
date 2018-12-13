#include <future>

#include "Auth.h"
#include "Network.h"

namespace auth {

  bool Auth::static_init() {
    instance_ = std::make_unique<Auth>();

    // トークンがあれば読み込む
    // ...

    return true;
  }

  void Auth::request_token(const std::string &id, const std::string &pw) {
    token_ = client::Network::instance_->token_request(id, pw);
  }

  bool Auth::token_exists() {
    return !token_.empty();
  }

} // namespace auth
