file(GLOB sources *.cpp)
file(GLOB headers *.h)
file(GLOB sources_command command/*.cpp)
file(GLOB headers_command command/*.h)
file(GLOB sources_protocol protocol/*.cpp)
file(GLOB headers_protocol protocol/*.h)

add_library(rudp STATIC
        ${sources} ${headers}
        ${sources_command} ${headers_command}
        ${sources_protocol} ${headers_protocol} RUdpCompress.cpp)
