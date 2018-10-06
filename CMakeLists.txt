cmake_minimum_required(VERSION 3.12)

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
        URL "https://github.com/ruslo/hunter/archive/v0.23.30.tar.gz"
        SHA1 "c062b0a2316cf8dcb57b6a9ad49fea1025a928f6"
)

project(life)
set(PROJECT_VERSION 0.0.1)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)
cable_add_buildinfo_library(PROJECT_NAME life)

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
else()
    message("License file: '${GTest_LICENSES}'")
endif()

#  spdlog
# --------------------------------------------------

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

#  Subdirectories
# --------------------------------------------------

add_subdirectory(engine/libnetwork)
add_subdirectory(engine/libutil)
add_subdirectory(life)