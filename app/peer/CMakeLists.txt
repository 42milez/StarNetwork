file(GLOB sources *.cc)
file(GLOB headers *.h)

add_executable(peer main.cc ${sources} ${headers})

set(libs
    p2p_techdemo-buildinfo
    core
    ${CONAN_LIBS})

target_link_libraries(peer PRIVATE ${libs})
