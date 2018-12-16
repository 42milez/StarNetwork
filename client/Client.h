#ifndef LIFE_LIFE_H
#define LIFE_LIFE_H

#include <memory>
#include <string>

namespace client {

  class Client {
  public:
    bool init();

    void run();

    void set_should_keep_running(bool should_keep_running);

  private:
    void do_run_loop();

    void do_frame();

    bool should_keep_running_;

    std::string request_token();
  };

} // namespace client

#endif // LIFE_LIFE_H
