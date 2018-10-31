file(GLOB sources *.cpp)
file(GLOB headers *.h)
add_library(util STATIC ${sources} ${headers})
