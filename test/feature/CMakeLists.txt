file(GLOB rudp_tests rudp/*.cc)

add_executable(feature_test
        main.cc
        ${rudp_tests})

target_link_libraries(feature_test
        core
        rudp
        module
        ${CONAN_LIBS})
