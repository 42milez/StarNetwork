#ifndef LIFE_LIFE_H
#define LIFE_LIFE_H

#include <memory>
#include <string>

#include <SDL.h>

namespace client {

  class Client {
  public:
    ~Client();

    static bool static_init();

    static std::unique_ptr<Client> instance_;

    virtual int run();

    void set_should_keep_running(bool should_keep_running);

    virtual void handle_event(SDL_Event *inEvent);

  private:
    Client();

    virtual void do_frame();

    int do_run_loop();

    bool should_keep_running_;

    std::string request_token();
  };

} // namespace client

#endif // LIFE_LIFE_H
