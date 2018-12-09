file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_library(base STATIC ${sources} ${headers} Worker.cpp Worker.h ../../client/Auth.cpp ../../client/Auth.h)

target_link_libraries(base PRIVATE spdlog::spdlog)
