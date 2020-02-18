set(sources NetworkTests.cc)

add_executable(network_test ${sources})

target_link_libraries(network_test core rudp module ${CONAN_LIBS})
