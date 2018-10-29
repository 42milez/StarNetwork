file(GLOB sources *.cpp)
file(GLOB headers *.h)
add_library(network STATIC ${sources} ${headers})
#target_link_libraries(network PRIVATE spdlog::spdlog)