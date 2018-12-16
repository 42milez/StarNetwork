#include <future>
#include <engine/base/Singleton.h>

#include "Auth.h"
#include "Network.h"

namespace auth {

//  bool Auth::static_init() {
//    instance_ = std::make_unique<Auth>();
//
//    // トークンがあれば読み込む
//    // ...
//
//    return true;
//  }

  bool Auth::init() {
    return true;
  }

  void Auth::request_token(const std::string &id, const std::string &pw) {
    auto network = engine::base::Singleton<client::Network>::Instance();
    token_ = network.token_request(id, pw);
  }

  bool Auth::token_exists() {
    return !token_.empty();
  }

} // namespace auth
