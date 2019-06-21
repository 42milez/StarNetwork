file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(module STATIC ${sources} ${headers})

target_link_libraries(module PRIVATE core udp)
