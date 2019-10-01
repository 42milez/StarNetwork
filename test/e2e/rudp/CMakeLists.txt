file(GLOB sources *.cpp)

add_executable(rudp_test ${sources})

target_link_libraries(rudp_test core rudp module Catch2::Catch2)
