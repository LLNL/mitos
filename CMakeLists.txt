project(perfsmpl C CXX)
cmake_minimum_required(VERSION 2.8)

# Include project-local cmake files.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package(Threads REQUIRED)
find_package(PerfEvents REQUIRED)
find_package(Dyninst REQUIRED SymtabAPI)

# Subdirectories
add_subdirectory(src)

