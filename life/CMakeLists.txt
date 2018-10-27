file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(life main.cpp ${sources} ${headers})

set(
  libs
  life-buildinfo
  Boost::program_options
  SDL2::SDL2main
  SDL2::SDL2
)

target_link_libraries(life PRIVATE ${libs})
