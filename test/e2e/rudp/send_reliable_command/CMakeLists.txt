file(GLOB sources *.cpp)

add_executable(send_reliable_command ${sources})

target_link_libraries(send_reliable_command PRIVATE core rudp module Catch2::Catch2)
