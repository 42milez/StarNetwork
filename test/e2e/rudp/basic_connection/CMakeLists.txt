file(GLOB sources *.cc)

add_executable(basic_connection_test ${sources})

target_link_libraries(basic_connection_test core rudp module ${CONAN_LIBS})
