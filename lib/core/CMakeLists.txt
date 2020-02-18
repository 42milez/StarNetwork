file(GLOB sources *.cc io/*.cc)
file(GLOB headers *.h io/*.h)

add_library(core STATIC ${sources} ${headers})

target_link_libraries(core INTERFACE ${CONAN_LIBS})
