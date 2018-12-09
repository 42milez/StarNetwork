file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(client main.cpp ${sources} ${headers})

set(libs
  life-buildinfo
  Boost::program_options
  SDL2::SDL2
  SDL2::SDL2main)

target_link_libraries(client PRIVATE ${libs})
