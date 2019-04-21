set(sources transporter_test.cpp)

add_executable(transporter_test ${sources})

target_link_libraries(transporter_test PRIVATE core udp networking Catch2::Catch2 ZLIB::zlib)
