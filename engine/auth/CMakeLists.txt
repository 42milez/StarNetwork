set(sources auth.cpp auth.h)

add_library(auth ${sources})

target_link_libraries(auth PRIVATE network spdlog::spdlog)
