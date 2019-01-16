#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/program_options.hpp>

#include "credential_generated.h"

#include "p2p_techdemo/buildinfo.h"
#include "engine/base/Singleton.h"
#include "Client.h"
#include "Network.h"

namespace po = boost::program_options;

namespace
{
  void
  version()
  {
    const auto *buildinfo = p2p_techdemo_get_buildinfo();
    std::cout << "p2p_techdemo " << buildinfo->project_version << "\n";
    std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
  }
} // namespace

int
main(int argc, char **argv)
{
  po::options_description opt_desc("OPTIONS", 160);
  opt_desc.add_options()
    ("run,r", "Execute the game client")
    ("help,h", "Show this help message and exit\n");

  po::options_description allowed_options("Allowed options");
  allowed_options.add(opt_desc);

  po::variables_map vmap;
  std::vector<std::string> unrecognized_options;

  try
  {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(allowed_options).allow_unregistered().run();
    unrecognized_options = collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vmap);
    po::notify(vmap);
  } catch (po::error const &e)
  {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  if (argc <= 1)
  {
    std::cout << "Requires at least 1 argument." << std::endl << std::endl;
    std::cout << "USAGE:" << std::endl
              << "  p2p_techdemo [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_FAILURE;
  }

  for (const auto &option : unrecognized_options)
  {
    std::cerr << "Invalid argument: " << option << "\n";
    return EXIT_FAILURE;
  }

  if (vmap.count("h") || vmap.count("help"))
  {
    std::cout << "NAME: " << std::endl
              << "  p2p_techdemo " << p2p_techdemo_get_buildinfo()->project_version << std::endl << std::endl
              << "USAGE:" << std::endl
              << "  p2p_techdemo [options]" << std::endl << std::endl;
    std::cout << opt_desc;
    return EXIT_SUCCESS;
  }
  else if (vmap.count("r") || vmap.count("run"))
  {
    auto &client = engine::base::Singleton<client::Client>::Instance();
    client.init();

    if (!client.token_exists())
    {
      //  ユーザー認証
      // --------------------------------------------------
      //  - Tokenを所持している、かつ、Tokenの有効期限が切れていなければ
      //    認証をスキップする

      // IDとパスワードの入力画面を描画（描画は別スレッドで実行する）
      // ...

      // ローディング画面を描画（描画は別スレッドで実行する）
      // ...

      // ユーザー認証
      // OPTIMIZE: 非同期処理にした方がよい？

      using namespace flatbuffers::credential;

      flatbuffers::FlatBufferBuilder builder(1024);

      std::string in_id;
      std::string in_pw;

      std::cout << "username: ";
      std::cin >> in_id;

      std::cout << "password: ";
      std::cin >> in_pw;
      std::cout << std::endl;

      // validation
      // ...

      auto id = builder.CreateString(in_id);
      auto password = builder.CreateString(in_pw);
      auto credential = CreateCredential(builder, id, password);

      builder.Finish(credential);

      auto *buf = builder.GetBufferPointer();
      auto size = builder.GetSize();

      client.request_token(buf, size);
    }

    client.run();
  }

} // namespace
