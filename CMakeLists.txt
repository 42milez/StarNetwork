cmake_minimum_required(VERSION 3.14.2)

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
cable_configure_toolchain(DEFAULT cxx17)
cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

include(HunterGate)
set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Hunter configuration types")
set(HUNTER_JOBS_NUMBER 4 CACHE STRING "Hunter jobs number")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.214.tar.gz"
    SHA1 "e14bc153a7f16d6a5eeec845fb0283c8fad8c358"
    LOCAL
)

project(P2P_TECHDEMO)
set(PROJECT_VERSION 0.0.1)
cable_add_buildinfo_library(PROJECT_NAME p2p_techdemo)

# https://stackoverflow.com/questions/6077414/how-to-set-the-ldflags-in-cmakelists-txt
# TODO: Indicate suitable path for each platform.
if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")
endif()

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0")

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.15")
endif()

if (APPLE)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
elseif()
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")

#  Boost
# --------------------------------------------------
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED)

#  Catch2
# --------------------------------------------------
hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)

#  spdlog
# --------------------------------------------------
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

#  Directories
# --------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(app)
add_subdirectory(lib/core)
add_subdirectory(lib/rudp)
add_subdirectory(module)

#  Directories (Tests)
# --------------------------------------------------
add_subdirectory(test/e2e/rudp/basic_connection)
add_subdirectory(test/e2e/rudp/broadcast)
add_subdirectory(test/e2e/rudp/send_reliable_command)
add_subdirectory(test/unit/core/io)
add_subdirectory(test/unit/module)
add_subdirectory(test/unit/lib/rudp/command)
