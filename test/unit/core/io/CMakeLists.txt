set(headers ../../../../core/io/ip_address.h)

set(sources ip_address_test.cpp
        ../../../../core/io/ip_address.cpp)

add_executable(ip_address_test ${headers} ${sources})

target_link_libraries(ip_address_test PRIVATE Catch2::Catch2)
