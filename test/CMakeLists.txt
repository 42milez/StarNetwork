file(GLOB sources
        feature/rudp/*.cc
        unit/core/io/*.cc
        unit/lib/rudp/command/*.cc
        unit/module/*.cc)

file(GLOB headers
        lib/test/util/*.h
        feature/rudp/*.h)

add_executable(all_tests
        main.cc
        ${sources}
        ${headers})

target_link_libraries(all_tests
        core
        rudp
        module
        ${CONAN_LIBS})
