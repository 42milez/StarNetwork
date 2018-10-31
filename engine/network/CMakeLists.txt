set(sources tcp_socket.h)

if (Linux)
  add_library(network ${sources} tcp_socket_linux.cpp)
else ()
  add_library(network ${sources} tcp_socket_unix.cpp)
endif ()

target_link_libraries(network PRIVATE util spdlog::spdlog)
