set(headers ${PROJECT_SOURCE_DIR}/module/networking/transporter.h)

set(sources transporter_test.cpp
            ${PROJECT_SOURCE_DIR}/module/networking/transporter.cpp)

add_executable(transporter_test ${headers} ${sources})

target_link_libraries(transporter_test PRIVATE core udp Catch2::Catch2 ZLIB::zlib)
