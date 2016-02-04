include_directories("..")

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_executable(api_test api_test.cpp)
add_executable(matmul matmul.cpp)

target_link_libraries(api_test mitos)

if(Dyninst_FOUND)
    include_directories(${DYNINST_INCLUDE_DIR})
endif()

if(MPI_FOUND)
    include_directories(${MPI_INCLUDE_PATH})
    add_executable(mpi_matmul mpi_matmul.cpp)
    target_link_libraries(mpi_matmul mitos mitoshooks ${MPI_LIBRARIES})
endif()

