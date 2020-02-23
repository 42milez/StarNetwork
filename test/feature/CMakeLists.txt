file(GLOB rudp_test_sources
        rudp/connection_test.cc
        rudp/broadcast_test.cc
        rudp/reliable_command.cc)

file(GLOB rudp_test_headers
        lib/test/util.h
        rudp/var.h)

add_executable(feature_test
        main.cc
        ${rudp_test_sources}
        ${rudp_test_headers})

target_link_libraries(feature_test
        core
        rudp
        module
        ${CONAN_LIBS})
