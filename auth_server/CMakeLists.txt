file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(auth_server main.cpp ${sources} ${headers})

set(libs
  p2p_techdemo-buildinfo
  Boost::program_options
  spdlog::spdlog
  base
  network)

target_link_libraries(auth_server PRIVATE ${libs})
