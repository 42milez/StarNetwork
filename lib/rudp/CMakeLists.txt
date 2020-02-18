file(GLOB sources *.cc)
file(GLOB headers *.h)
file(GLOB sources_command command/*.cc)
file(GLOB headers_command command/*.h)
file(GLOB sources_dispatch dispatch/*.cc)
file(GLOB headers_dispatch dispatch/*.h)
file(GLOB headers_peer peer/*.h)
file(GLOB sources_peer peer/*.cc)
file(GLOB sources_protocol protocol/*.cc protocol/*.cc)
file(GLOB headers_protocol protocol/*.h)

add_library(rudp STATIC
        ${sources} ${headers}
        ${sources_command} ${headers_command}
        ${sources_dispatch} ${headers_dispatch}
        ${sources_peer} ${headers_peer}
        ${sources_protocol} ${headers_protocol})

target_link_libraries(rudp INTERFACE core)
