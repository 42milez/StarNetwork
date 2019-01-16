#ifndef P2P_TECHDEMO_CLIENT_H
#define P2P_TECHDEMO_CLIENT_H

#include <memory>
#include <string>

namespace client {

  class Client {
  public:
    bool init();

    void run();

    int request_token(const uint8_t *buf, uint32_t size);

    bool token_exists();

  private:
    std::string token_;
  };

} // namespace client

#endif // P2P_TECHDEMO_CLIENT_H
