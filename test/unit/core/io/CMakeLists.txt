set(headers ${P2P_TECHDEMO_SOURCE_DIR}/lib/core/io/ip_address.h)
set(sources ip_address_test.cpp ${P2P_TECHDEMO_SOURCE_DIR}/lib/core/io/ip_address.cpp)

add_executable(ip_address_test ${headers} ${sources})

target_link_libraries(ip_address_test PRIVATE Catch2::Catch2)
