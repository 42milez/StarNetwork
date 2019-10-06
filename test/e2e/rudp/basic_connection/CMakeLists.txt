file(GLOB sources *.cpp)

add_executable(basic_connection_test ${sources})

target_link_libraries(basic_connection_test PRIVATE core rudp module Catch2::Catch2)
