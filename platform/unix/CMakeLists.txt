file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(unix STATIC ${sources} ${headers})
