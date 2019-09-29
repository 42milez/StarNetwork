#include <iostream>
#include <vector>

#include "p2p_techdemo/buildinfo.h"

namespace
{
  void
  version()
  {
    const auto *buildinfo = p2p_techdemo_get_buildinfo();
    std::cout << "p2p_techdemo " << buildinfo->project_version << "\n";
    std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
  }

  const std::string LOGGER_NAME = "PEER";
  const std::string PATH_LOG = "/var/log/p2p_techdemo/peer.log";
} // namespace

int
main(int argc, char **argv)
{
    //  po::options_description opt_desc("OPTIONS", 160);
    //  opt_desc.add_options()
    //    ("run,r", "Execute the game peer")
    //    ("help,h", "Show this help message and exit\n");

    //  po::options_description allowed_options("Allowed options");
    //  allowed_options.add(opt_desc);

    //  po::variables_map vmap;
    //  std::vector<std::string> unrecognized_options;

    //  try
    //  {
    //    po::parsed_options parsed = po::command_line_parser(argc, argv).options(allowed_options).allow_unregistered().run();
    //    unrecognized_options = collect_unrecognized(parsed.options, po::include_positional);
    //    po::store(parsed, vmap);
    //    po::notify(vmap);
    //  } catch (po::error const &e)
    //  {
    //    std::cerr << e.what();
    //    return EXIT_FAILURE;
    //  }

    //  if (argc <= 1)
    //  {
    //    std::cout << "Requires at least 1 argument." << std::endl << std::endl;
    //    std::cout << "USAGE:" << std::endl
    //              << "  p2p_techdemo [options]" << std::endl << std::endl;
    //    std::cout << opt_desc;
    //    return EXIT_FAILURE;
    //  }

    //  for (const auto &option : unrecognized_options)
    //  {
    //    std::cerr << "Invalid argument: " << option << "\n";
    //    return EXIT_FAILURE;
    //  }

    //  if (vmap.count("h") || vmap.count("help"))
    //  {
    //    std::cout << "NAME: " << std::endl
    //              << "  p2p_techdemo " << p2p_techdemo_get_buildinfo()->project_version << std::endl << std::endl
    //              << "USAGE:" << std::endl
    //              << "  p2p_techdemo [options]" << std::endl << std::endl;
    //    std::cout << opt_desc;
    //    return EXIT_SUCCESS;
    //  }
    //  else if (vmap.count("r") || vmap.count("run"))
    //  {
    //    auto &logger = core::Singleton<core::Logger>::Instance();
    //
    //    if (!logger.init(LOGGER_NAME, PATH_LOG)) {
    //      std::cout << "Initialization failed." << std::endl;
    //      return EXIT_FAILURE;
    //    }

    //    auto &peer = core::Singleton<peer::Peer>::Instance();
    //    peer.init();

    //    if (!peer.token_exists())
    //    {
    //      // ユーザー認証
    //      // ...
    //    }

    //    peer.run();
    //  }

} // namespace
