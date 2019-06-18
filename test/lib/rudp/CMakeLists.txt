set(sources host_test.cpp)

add_executable(host_test ${sources})

target_link_libraries(host_test PRIVATE Catch2::Catch2)
