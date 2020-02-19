set(E2E_TEST_RUDP_BASE_LIBS core rudp module ${CONAN_LIBS})

# Basic Connection Test
add_executable(basic_connection_test ${CMAKE_CURRENT_SOURCE_DIR}/rudp/basic_connection.cc)
target_link_libraries(basic_connection_test ${E2E_TEST_RUDP_BASE_LIBS})

# Broadcast Test
add_executable(broadcast_test ${CMAKE_CURRENT_SOURCE_DIR}/rudp/broadcast.cc)
target_link_libraries(broadcast_test ${E2E_TEST_RUDP_BASE_LIBS})

# Reliable Command Test
add_executable(reliable_command_test ${CMAKE_CURRENT_SOURCE_DIR}/rudp/reliable_command.cc)
target_link_libraries(reliable_command_test ${E2E_TEST_RUDP_BASE_LIBS})
