include_directories(${MPI_INCLUDE_PATH})
include_directories(${DYNINST_INCLUDE_DIR})
include_directories("..")

set(CMAKE_BUILD_TYPE Debug)

add_executable(test test.cpp)
add_executable(matmul matmul.cpp)
add_executable(mpi_matmul mpi_matmul.cpp)

target_link_libraries(test mitos symtabAPI)
target_link_libraries(mpi_matmul ${MPI_LIBRARIES})

