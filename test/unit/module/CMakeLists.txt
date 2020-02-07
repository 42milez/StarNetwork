set(sources NetworkTests.cpp)

add_executable(network_test ${sources})

target_link_libraries(network_test PRIVATE core rudp module Catch2::Catch2)
