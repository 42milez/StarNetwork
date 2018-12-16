#ifndef LIFE_AUTH_H
#define LIFE_AUTH_H

#include <memory>
#include <string>

namespace auth {

  class Auth {
  public:
//    static bool static_init();

    bool init();

    void request_token(const std::string &id, const std::string &password);

    bool token_exists();

//    static std::unique_ptr<Auth> instance_;
  private:
    Auth() = default;

    std::string token_;
  };

} // namespace auth

#endif // LIFE_AUTH_H
