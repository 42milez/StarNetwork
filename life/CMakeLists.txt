add_executable(life main.cpp)
target_link_libraries(life PRIVATE life-buildinfo Boost::program_options)
