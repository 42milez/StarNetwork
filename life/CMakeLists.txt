file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(life main.cpp ${sources} ${headers} ../engine/auth/auth.cpp ../engine/auth/auth.h)

set(libs
  life-buildinfo
  Boost::program_options
  SDL2::SDL2main
  SDL2::SDL2
  auth
  base
  )

target_link_libraries(life PRIVATE ${libs})
