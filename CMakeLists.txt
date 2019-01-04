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
  URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
  SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
  LOCAL
)

project(p2p_techdemo)

set(PROJECT_VERSION 0.0.1)

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

#  GTest
# --------------------------------------------------

hunter_add_package(GTest)

find_package(GTest CONFIG REQUIRED)
find_package(GMock CONFIG REQUIRED)

if (NOT EXISTS "${GTest_LICENSES}")
  message(FATAL_ERROR "File not found: '${GTest_LICENSES}")
else ()
  message("License file: '${GTest_LICENSES}'")
endif ()

#  spdlog
# --------------------------------------------------

hunter_add_package(spdlog)

find_package(spdlog CONFIG REQUIRED)

#  Directories
# --------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(engine/base)
add_subdirectory(engine/network)
add_subdirectory(client)
add_subdirectory(auth_server)
