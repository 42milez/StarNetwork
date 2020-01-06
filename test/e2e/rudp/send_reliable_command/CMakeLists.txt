file(GLOB sources *.cpp)

add_executable(send_reliable_command_test ${sources})

target_link_libraries(send_reliable_command_test PRIVATE core rudp module Catch2::Catch2)
