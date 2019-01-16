#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "p2p_techdemo/buildinfo.h"

#include "engine/base/Logger.h"
#include "engine/base/Singleton.h"

#include "auth_server/AuthServer.h"
#include "auth_server/Network.h"

namespace po = boost::program_options;

namespace
{
  void version() {
    const auto *buildinfo = p2p_techdemo_get_buildinfo();
    std::cout << "p2p_techdemo " << buildinfo->project_version << "\n";
    std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
  }

  const std::string LOGGER_NAME = "AUTH_SERVER";
  const std::string PATH_LOG = "/var/log/p2p_techdemo/auth_server.log";
} // namespace

int main(int argc, char **argv) {
  po::options_description opt_desc("OPTIONS", 160);
  opt_desc.add_options()
    ("run,r", "Execute the server")
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
              << "  auth_server [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_FAILURE;
  }

  for (const auto &option : unrecognized_options) {
    std::cerr << "Invalid argument: " << option << "\n";
    return EXIT_FAILURE;
  }

  if (vmap.count("h") || vmap.count("help")) {
    std::cout << "NAME: " << std::endl
              << "  auth_server " << p2p_techdemo_get_buildinfo()->project_version << std::endl << std::endl
              << "USAGE:" << std::endl
              << "  auth_server [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_SUCCESS;
  } else if (vmap.count("r") || vmap.count("run")) {
    // todo: handle exit with std::async (can it?)
    // ...

    auto &logger = engine::base::Singleton<engine::base::Logger>::Instance();

    if (!logger.init(LOGGER_NAME, PATH_LOG)) {
      std::cout << "Initialization failed." << std::endl;
      return EXIT_FAILURE;
    }

    auto &auth_server = engine::base::Singleton<auth_server::AuthServer>::Instance();

    if (!auth_server.init()) {
      std::cout << "Initialization failed." << std::endl;
      return EXIT_FAILURE;
    }

    auth_server.run();

    engine::base::SingletonFinalizer::finalize();

    return EXIT_SUCCESS;
  }

} // namespace
