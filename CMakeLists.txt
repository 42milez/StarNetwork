cmake_minimum_required(VERSION 3.12.3)

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
  URL "https://github.com/ruslo/hunter/archive/v0.23.88.tar.gz"
  SHA1 "b7b40b35f96fc06d5f027001160c4e83f7576859"
  LOCAL
)

project(p2p_techdemo)

set(PROJECT_VERSION 0.0.1)

# https://stackoverflow.com/questions/6077414/how-to-set-the-ldflags-in-cmakelists-txt
# TODO: Indicate suitable path for each platform.
set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")

set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

cable_add_buildinfo_library(PROJECT_NAME p2p_techdemo)

#  OpenSSL
# --------------------------------------------------

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

#  Boost
# --------------------------------------------------

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

#  FlatBuffers
# --------------------------------------------------

hunter_add_package(flatbuffers)

find_package(Flatbuffers CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/flatbuffers/include)

#  Directories
# --------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(core/base)
add_subdirectory(core/io)

add_subdirectory(platform/unix)

add_subdirectory(engine/base)
add_subdirectory(engine/network)

add_subdirectory(client)
add_subdirectory(auth_server)

#  Directories (Test)
# --------------------------------------------------

add_subdirectory(test/core/io)
