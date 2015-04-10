project(Mitos C CXX)
cmake_minimum_required(VERSION 2.8)

# Version info
set(MITOS_MAJOR_VERSION 0)
set(MITOS_MINOR_VERSION 8)
set(MITOS_PATCH_VERSION 1)
set(MITOS_VERSION ${MITOS_MAJOR_VERSION}.${MITOS_MINOR_VERSION}.${MITOS_PATCH_VERSION})

# Included cmake files
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# Packages required
find_package(Threads REQUIRED)
find_package(PerfEvents REQUIRED)
find_package(MPI REQUIRED)
find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED SymtabAPI InstructionAPI ParseAPI)
find_package(OpenMP)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Subdirectories
add_subdirectory(src)

# Export the package
export(PACKAGE Mitos)

# Install cmake export
install(EXPORT mitos-targets DESTINATION lib/Mitos)
install(FILES mitos-config.cmake DESTINATION lib/Mitos)

