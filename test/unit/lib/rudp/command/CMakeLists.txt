file(GLOB sources *.cc)

add_executable(rudp_command_test ${sources})

target_link_libraries(rudp_command_test core rudp module ${CONAN_LIBS})
