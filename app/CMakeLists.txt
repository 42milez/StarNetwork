file(GLOB sources *.cc)
file(GLOB headers *.h)

add_executable(star_network main.cc ${sources} ${headers})

set(libs
        star_network-buildinfo
        core
        rudp
        ${CONAN_LIBS})

target_link_libraries(star_network ${libs})
