#ifndef P2P_TECHDEMO_CLIENT_H
#define P2P_TECHDEMO_CLIENT_H

#include <memory>
#include <string>

namespace client {

  class Client {
  public:
    Client();

    bool init();

    void run();

    void set_should_keep_running(bool should_keep_running);

    int request_token(const uint8_t *buf, uint32_t size);

    bool token_exists();

  private:
    void do_frame();

    bool should_keep_running_;

    std::string token_;
  };

} // namespace client

#endif // P2P_TECHDEMO_CLIENT_H
