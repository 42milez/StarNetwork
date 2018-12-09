#include <ctime>
#include <string>

#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"
#include "Client.h"
#include "Network.h"

namespace client {

  bool Client::static_init() {
    instance_ = std::make_unique<Client>();
    return true;
  }

  Client::Client() : should_keep_running_(true) {
    srand(static_cast<uint32_t>(time(nullptr)));

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    std::string destination = "192.168.0.1";

    network::SocketAddressPtr server_address = network::SocketAddressFactory::create_ipv4_from_string(destination);

    Network::static_init(*server_address);
  }

  Client::~Client() {
    SDL_Quit();
  }

  std::string Client::request_token() {
    return "";
  }

  int Client::run() {
    return do_run_loop();
  }

  void Client::set_should_keep_running(bool should_keep_running) {
    should_keep_running_ = should_keep_running;
  }

  void Client::handle_event(SDL_Event *inEvent) {
    (void) inEvent;
  }

  int Client::do_run_loop() {
    bool quit = false;
    SDL_Event event;
    memset(&event, 0, sizeof(SDL_Event));

    while (!quit && should_keep_running_) {
      if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          quit = true;
        } else {
          handle_event(&event);
        }
      } else {
        // todo
        // Timing::sInstance.Update();
        do_frame();
      }
    }

    return event.type;
  }

  void Client::do_frame() {
    // ...
  }

} // namespace client
