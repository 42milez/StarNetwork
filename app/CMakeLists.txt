file(GLOB sources *.cc)
file(GLOB headers *.h)

add_executable(p2p_techdemo main.cc ${sources} ${headers})

set(libs
    p2p_techdemo-buildinfo
    core
    ${CONAN_LIBS})

target_link_libraries(p2p_techdemo ${libs})
