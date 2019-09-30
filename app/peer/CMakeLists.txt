file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(peer main.cpp ${sources} ${headers})

set(libs
  p2p_techdemo-buildinfo
  core
  spdlog::spdlog)

target_link_libraries(peer PRIVATE ${libs})
