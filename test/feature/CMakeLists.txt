file(GLOB rudp_test_sources
        rudp/basic_connection.cc
        rudp/broadcast.cc
        rudp/reliable_command.cc)

file(GLOB rudp_test_headers rudp/var.h)

add_executable(feature_test
        main.cc
        ${rudp_test_sources}
        ${rudp_test_headers})

target_link_libraries(feature_test
        core
        rudp
        module
        ${CONAN_LIBS})
