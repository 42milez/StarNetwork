set(sources RUdpHostTests.cpp)

add_executable(rudp_host_test ${sources})

target_link_libraries(rudp_host_test PRIVATE core rudp module Catch2::Catch2)
