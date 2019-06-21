file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(rudp STATIC ${sources} ${headers})
