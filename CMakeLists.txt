cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

project(spglib)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)

option(BUILD_SHARED_LIBS "Build with shared libraries" ON)

add_subdirectory(src)
