file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(client main.cpp ${sources} ${headers})

set(libs
  p2p_techdemo-buildinfo
  spdlog::spdlog
  core
  network)

target_link_libraries(client PRIVATE ${libs})
