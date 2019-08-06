cmake_minimum_required(VERSION 3.14.2)

#  Configurations
# --------------------------------------------------

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)

cable_configure_toolchain(DEFAULT cxx17)

set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Hunter configuration types")
set(HUNTER_JOBS_NUMBER 4 CACHE STRING "Hunter jobs number")

include(HunterGate)

HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.155.tar.gz"
  SHA1 "6421d63cc4fd78b8622e863c6b1dcabce98e1216"
  LOCAL
)

project(p2p_techdemo)

set(PROJECT_VERSION 0.0.1)

# https://stackoverflow.com/questions/6077414/how-to-set-the-ldflags-in-cmakelists-txt
# TODO: Indicate suitable path for each platform.
set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")

set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

cable_add_buildinfo_library(PROJECT_NAME p2p_techdemo)

#  OpenSSL
# --------------------------------------------------

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

#  Boost
# --------------------------------------------------

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

hunter_add_package(Boost COMPONENTS program_options)

find_package(Boost CONFIG REQUIRED COMPONENTS program_options)

#  Catch2
# --------------------------------------------------

hunter_add_package(Catch)

find_package(Catch2 CONFIG REQUIRED)

#  spdlog
# --------------------------------------------------

hunter_add_package(spdlog)

find_package(spdlog CONFIG REQUIRED)

#  zlib
# --------------------------------------------------

hunter_add_package(ZLIB)

find_package(ZLIB CONFIG REQUIRED)

#  Directories
# --------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(core)
add_subdirectory(lib/rudp)
add_subdirectory(module)

add_subdirectory(engine/base)
add_subdirectory(engine/network)

add_subdirectory(client)
add_subdirectory(auth_server)

#  Directories (Test)
# --------------------------------------------------

add_subdirectory(test/core/io)
add_subdirectory(test/lib/rudp)
add_subdirectory(test/module)
