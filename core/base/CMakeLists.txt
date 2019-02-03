file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(base2 STATIC ${sources} ${headers})
