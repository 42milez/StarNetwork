#ifndef LIFE_LIFE_H
#define LIFE_LIFE_H

#include <memory>
#include <string>

namespace client {

  class Client {
  public:
    Client();

    bool init();

    void run();

    void set_should_keep_running(bool should_keep_running);

    void request_token(const std::string &id, const std::string &password);

    bool token_exists();

  private:
    void do_run_loop();

    void do_frame();

    bool should_keep_running_;

    std::string token_;
  };

} // namespace client

#endif // LIFE_LIFE_H
