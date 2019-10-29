file(GLOB sources *.cpp)

add_executable(reliable_command ${sources})

target_link_libraries(reliable_command PRIVATE core rudp module Catch2::Catch2)
