#  CORE
# --------------------------------------------------

# core/io/ip_address
add_executable(ip_address_test ${CMAKE_CURRENT_SOURCE_DIR}/core/io/ip_address_test.cc)
target_link_libraries(ip_address_test core ${CONAN_LIBS})

#  LIB
# --------------------------------------------------

# lib/rudp/command_pod
add_executable(command_pod_test ${CMAKE_CURRENT_SOURCE_DIR}/lib/rudp/command/command_pod_test.cc)
target_link_libraries(command_pod_test core rudp module ${CONAN_LIBS})

#  MODULE
# --------------------------------------------------

# module/network
add_executable(network_test ${CMAKE_CURRENT_SOURCE_DIR}/module/network_test.cc)
target_link_libraries(network_test core rudp module ${CONAN_LIBS})
