file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(networking STATIC ${sources} ${headers})

target_link_libraries(networking PRIVATE core udp)
