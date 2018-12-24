#include "ExitHandler.h"

namespace engine
{
  namespace util
  {
    bool ExitHandler::should_exit_ = false;

    void ExitHandler::init() {
      // Handle Exit Signal
      auto sig_handler = [](int signum) { ExitHandler::exit(); };
      register_handler(SIGABRT, sig_handler);
      register_handler(SIGINT, sig_handler);
      register_handler(SIGTERM, sig_handler);

      // Handle Pipe Signal
      register_handler(SIGPIPE, SIG_IGN);
    }

    void ExitHandler::exit() {
      should_exit_ = true;
    }

    template<typename Func>
    void ExitHandler::register_handler(int signum, Func handler) {
      struct sigaction act{nullptr};
      act.sa_handler = handler;
      act.sa_flags = 0;

      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, signum);

      act.sa_mask = set;

      sigaction(signum, nullptr, &act);
    }

    bool ExitHandler::should_exit() const {
      return should_exit_;
    }
  } // namespace util
} // namespace engine
