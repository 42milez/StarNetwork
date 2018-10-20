file(GLOB sources *.cpp)
file(GLOB headers *.h)
file(GLOB qmls *.ui)

add_executable(life main.cpp ${sources} ${headers} ${qmls})

set(
  libs
  life-buildinfo
  Boost::program_options
  Qt5::Widgets
  Qt5::Quick
)

if (APPLE)
  target_link_libraries(life PRIVATE ${libs} Qt5::PrintSupport)
else ()
  target_link_libraries(life PRIVATE ${libs})
endif ()
