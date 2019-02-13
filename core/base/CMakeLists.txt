file(GLOB sources *.cpp)
file(GLOB headers *.h)

# ToDo: rename base2 base
add_library(base2 STATIC ${sources} ${headers})
