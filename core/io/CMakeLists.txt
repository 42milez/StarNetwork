file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(io STATIC ${sources} ${headers})
