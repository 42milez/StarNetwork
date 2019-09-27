file(GLOB sources *.cpp)
file(GLOB headers *.h)
file(GLOB sources_command command/*.cpp)
file(GLOB headers_command command/*.h)
file(GLOB sources_dispatch dispatch/*.cpp)
file(GLOB headers_peer peer/*.h)
file(GLOB sources_peer peer/*.cpp)
file(GLOB headers_dispatch dispatch/*.h)
file(GLOB sources_protocol protocol/*.cpp)
file(GLOB headers_protocol protocol/*.h)

add_library(rudp STATIC
        ${sources} ${headers}
        ${sources_command} ${headers_command}
        ${sources_dispatch} ${headers_dispatch}
        ${sources_peer} ${headers_peer}
        ${sources_protocol} ${headers_protocol})
