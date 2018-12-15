file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(auth_server main.cpp ${sources} ${headers})

set(libs
  life-buildinfo
  Boost::program_options)

target_link_libraries(auth_server PRIVATE ${libs})
