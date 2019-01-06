file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(base STATIC ${sources} ${headers})

target_link_libraries(base PRIVATE spdlog::spdlog libc++fs.a)
