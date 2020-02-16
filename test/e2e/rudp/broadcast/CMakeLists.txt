file(GLOB sources *.cc)

add_executable(broadcast_test ${sources})

target_link_libraries(broadcast_test PRIVATE core rudp module ${CONAN_LIBS})
