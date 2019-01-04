file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(client main.cpp ${sources} ${headers})

set(libs
  p2p_techdemo-buildinfo
  Boost::program_options
  spdlog::spdlog
  base
  network)

target_link_libraries(client PRIVATE ${libs})
