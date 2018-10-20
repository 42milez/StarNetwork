#include <csignal>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <QApplication>

#include "life/buildinfo.h"
#include "MainWindow.h"

namespace po = boost::program_options;

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
  po::options_description opt_desc("OPTIONS", 160);
  opt_desc.add_options()
    ("run,r", "Execute the game client")
    ("help,h", "Show this help message and exit\n");

  po::options_description allowed_options("Allowed options");
  allowed_options.add(opt_desc);

  po::variables_map vmap;
  std::vector<std::string> unrecognized_options;

  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(allowed_options).allow_unregistered().run();
    unrecognized_options = collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vmap);
    po::notify(vmap);
  } catch (po::error const &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  if (argc <= 1) {
    std::cout << "Requires at least 1 argument." << std::endl << std::endl;
    std::cout << "USAGE:" << std::endl
              << "   life [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_FAILURE;
  }

  for (const auto &option : unrecognized_options) {
    std::cerr << "Invalid argument: " << option << "\n";
    return EXIT_FAILURE;
  }

  if (vmap.count("h") || vmap.count("help")) {
    std::cout << "NAME: " << std::endl
              << "   life " << life_get_buildinfo()->project_version << std::endl << std::endl
              << "USAGE:" << std::endl
              << "    life [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_SUCCESS;
  } else if (vmap.count("r") || vmap.count("run")) {
    // Handle Exit Signal
    auto sig_handler = [](int signum) { ExitHandler::exit(); };
    register_handler(SIGABRT, sig_handler);
    register_handler(SIGINT, sig_handler);
    register_handler(SIGTERM, sig_handler);

    // Handle Pipe Signal
    register_handler(SIGPIPE, SIG_IGN);

    ExitHandler eh;

    // todo: handle exit with std::async (can it?)
    // ...

    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    return app.exec();
  }
}
