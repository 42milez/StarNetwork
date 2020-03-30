#include <iostream>
#include <memory>

#include <lyra/lyra.hpp>

#include "lib/core/async_worker.h"
#include "lib/core/exit_handler.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "p2p_techdemo/buildinfo.h"

#include "network.h"

namespace
{
    void
    version()
    {
        const auto *buildinfo = p2p_techdemo_get_buildinfo();
        std::cout << "p2p_techdemo " << buildinfo->project_version << "\n";
        std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
    }

    const std::string DEFAULT_MODE           = "server";
    const std::string DEFAULT_SERVER_ADDRESS = "::FFFF:127.0.0.1";
    const uint16_t DEFAULT_PORT              = 49152;
    const std::string LOG_FILE_PATH          = "/var/log/p2p_techdemo/app.log";
} // namespace

int
main(int argc, const char **argv)
{
    if (!core::Singleton<core::ExitHandler>::Instance().Init()) {
        core::Singleton<core::Logger>::Instance().Critical("register signal handler failed");
        return -1;
    }

    std::string mode         = DEFAULT_MODE;
    int port                 = DEFAULT_PORT;
    std::string host_address = DEFAULT_SERVER_ADDRESS;
    int host_port            = DEFAULT_PORT;
    bool show_version        = false;
    bool show_help           = false;
    std::string log_file_path{};

    auto cli = lyra::cli_parser();
    cli.add_argument(
        lyra::opt(mode, "mode").name("-m").name("--mode").help("Run mode").choices("client", "server").optional());
    cli.add_argument(lyra::opt(port, "port")
                         .name("-p")
                         .name("--port")
                         .help("Port to bind")
                         .choices([](int value) { return 0 <= value && value <= 65535; })
                         .optional());
    cli.add_argument(
        lyra::opt(host_address, "host_address").name("-ha").name("--host-address").help("host address").optional());
    cli.add_argument(lyra::opt(host_port, "host_port").name("-hp").name("--host-port").help("host port").optional());
    cli.add_argument(lyra::opt(log_file_path, "log_file_path").name("-l").name("--log").help("log file path").optional());
    cli.add_argument(lyra::opt(show_version).name("-v").name("--version").help("Show application version").optional());
    cli.add_argument(lyra::help(show_help));

    auto result = cli.parse({argc, argv});
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        return 1;
    }

    if (show_version) {
        version();
        return 0;
    }

    if (show_help) {
        std::cout << cli;
        return 0;
    }

    if (!log_file_path.empty()) {
        core::Singleton<core::Logger>::Instance().EnableFileLogger(log_file_path);
    }

    auto node = std::make_shared<app::Network>();

    core::AsyncWorker node_runner{[&node]{
      node->Poll();
    }};

    node_runner.Run();

    core::AsyncWorker message_dispatcher{[&node]{
      if (node->Peek() > 0) {
          auto ret = node->Receive();

          // process message
          // ...
      }
    }};

    message_dispatcher.Run();

    if (mode == "server") {
        core::Singleton<core::Logger>::Instance().Info("running as server");
        node->CreateServer(port, 32, 100, 100);
    }
    else {
        core::Singleton<core::Logger>::Instance().Info("running as client");
        node->CreateClient(host_address, host_port, port, 100, 100);
    }

    while (!core::Singleton<core::ExitHandler>::Instance().ShouldExit()) {
        std::string message;
        std::cin >> message;

        if (message == "exit") {
            core::Singleton<core::ExitHandler>::Instance().Exit();
        }

        node->Send(message);
    }

    message_dispatcher.Stop();

    node_runner.Stop();

    return 0;
}
