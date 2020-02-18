file(GLOB sources *.cc)

add_executable(ip_address_test ${sources})

target_link_libraries(ip_address_test core ${CONAN_LIBS})
