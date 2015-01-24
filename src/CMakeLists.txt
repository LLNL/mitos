set(SRC_FILES perfsmpl.cpp mattr.cpp Mitos.cpp)
set(HDR_FILES perfsmpl.h mattr.h Mitos.h)

# Add shared and static libs
add_library(mitos SHARED ${SRC_FILES})

include_directories(${MPI_INCLUDE_PATH})

add_executable(test test.cpp)
add_executable(mpi_hello mpi_hello.c)
add_executable(mitosrun mitosrun.cpp)
add_executable(mitosprocess mitosprocess.cpp)

target_link_libraries(mitos ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test mitos symtabAPI)
target_link_libraries(mpi_hello mitos ${MPI_LIBRARIES})
target_link_libraries(mitosrun mitos)
target_link_libraries(mitosprocess mitos symtabAPI)

include_directories(${DYNINST_INCLUDE_DIR})

# Install mitosrun, mitosprocess, and the libraries.
install(TARGETS mitos DESTINATION lib)
install(TARGETS mitosrun DESTINATION bin)
install(TARGETS mitosprocess DESTINATION bin)
