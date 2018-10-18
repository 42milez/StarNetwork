file(GLOB sources *.cpp)
file(GLOB headers *.h)
file(GLOB qmls *.ui)

add_executable(life ${sources} ${headers} ${qmls})

target_link_libraries(life PRIVATE life-buildinfo Boost::program_options Qt5::Widgets)

if (APPLE)
  find_package(Qt5PrintSupport REQUIRED)
  target_link_libraries(life Qt5::PrintSupport)
endif ()
