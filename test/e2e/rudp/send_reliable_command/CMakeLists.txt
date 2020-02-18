file(GLOB sources *.cc)

add_executable(send_reliable_command_test ${sources})

target_link_libraries(send_reliable_command_test core rudp module ${CONAN_LIBS})
