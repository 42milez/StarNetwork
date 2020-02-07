file(GLOB sources *.cpp)

add_executable(rudp_command_test ${sources})

target_link_libraries(rudp_command_test PRIVATE core rudp module Catch2::Catch2)
