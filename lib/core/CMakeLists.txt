file(GLOB sources *.cpp io/*.cpp)
file(GLOB headers *.h io/*.h)

add_library(core STATIC ${sources} ${headers})

target_link_libraries(core spdlog::spdlog)
