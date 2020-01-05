file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(session main.cpp ${sources} ${headers})

set(libs
    p2p_techdemo-buildinfo
    spdlog::spdlog
    core)

target_link_libraries(session PRIVATE ${libs})
