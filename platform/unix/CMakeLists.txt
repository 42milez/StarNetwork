file(GLOB sources io/*.cpp)
file(GLOB headers io/*.h)

add_library(unix STATIC ${sources} ${headers})
