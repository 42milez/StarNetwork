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
    std::promise<void> p;

    std::string token = "";

    auto auth_thread = std::thread([this, &id, &pw, &p] {
      token_ = client::Network::instance_->token_request(id, pw);
      p.set_value();
    });

    p.get_future().wait();

    auth_thread.join();
  }

  bool Auth::token_exists() {
    return !token_.empty();
  }

} // namespace auth
