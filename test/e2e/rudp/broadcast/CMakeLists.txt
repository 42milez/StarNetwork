file(GLOB sources *.cpp)

add_executable(broadcast_test ${sources})

target_link_libraries(broadcast_test PRIVATE core rudp module Catch2::Catch2)
