#include <csignal>
#include <iostream>

#include "life/buildinfo.h"

namespace
{
  void version() {
    const auto* buildinfo = life_get_buildinfo();
    std::cout << "life " << buildinfo->project_version << "\n";
    std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
  }

  class ExitHandler {
  public:
    static void exit() { should_exit_ = true; }
    bool should_exit() const { return should_exit_; }
  private:
    static bool should_exit_;
  };

  bool ExitHandler::should_exit_ = false;

  template <typename Func>
  void register_handler(int signum, Func handler) {
    struct sigaction act { nullptr };
    act.sa_handler = handler;
    act.sa_flags = 0;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signum);

    act.sa_mask = set;

    sigaction(signum, nullptr, &act);
  }
} // namespace

int main(int argc, char** argv)
{
  // Handle Exit Signal
  auto sig_handler = [](int signum) { ExitHandler::exit(); };
  register_handler(SIGABRT, sig_handler);
  register_handler(SIGINT, sig_handler);
  register_handler(SIGTERM, sig_handler);

  // Handle Pipe Signal
  register_handler(SIGPIPE, SIG_IGN);

  return EXIT_SUCCESS;
}
