file(GLOB sources
        ${CMAKE_SOURCE_DIR}/app/network.cc
        ${CMAKE_SOURCE_DIR}/app/peer.cc
        feature/rudp/*.cc
        unit/app/*.cc
        unit/core/io/*.cc
        unit/lib/rudp/command/*.cc)

file(GLOB headers
        ${CMAKE_SOURCE_DIR}/app/*.h
        ${CMAKE_SOURCE_DIR}/lib/test/util/*.h
        feature/rudp/*.h)

add_executable(all_tests
        main.cc
        ${sources}
        ${headers})

target_link_libraries(all_tests
        core
        rudp
        ${CONAN_LIBS})
