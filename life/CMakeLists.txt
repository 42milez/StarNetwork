file(GLOB sources *.cpp)
file(GLOB headers *.h)
file(GLOB qmls *.ui)

add_executable(life main.cpp ${sources} ${headers} ${qmls})

#add_executable(life main.cpp)

target_link_libraries(life PRIVATE life-buildinfo Boost::program_options Qt5::Widgets)

#target_link_libraries(life PRIVATE life-buildinfo Boost::program_options)