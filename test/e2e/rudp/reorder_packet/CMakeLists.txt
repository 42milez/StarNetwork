file(GLOB sources *.cpp)

add_executable(packet_loss_test ${sources})

target_link_libraries(packet_loss_test PRIVATE core rudp module Catch2::Catch2)
