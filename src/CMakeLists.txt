add_subdirectory(examples)

set(MITOS_SRC_FILES perfsmpl.cpp mattr.cpp Mitos.cpp)
set(MITOS_MPIHOOK_SRC_FILES mitosmpihook.cpp)

add_library(mitos SHARED ${MITOS_SRC_FILES})
add_library(mitosmpihook SHARED ${MITOS_SRC_FILES} ${MITOS_MPIHOOK_SRC_FILES})

include_directories(${MPI_INCLUDE_PATH})
include_directories(${DYNINST_INCLUDE_DIR})

add_executable(mitosrun mitosrun.cpp)
add_executable(mitosmpirun mitosmpirun.cpp)
add_executable(mitosprocess mitosprocess.cpp)

target_link_libraries(mitos ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(mitosmpihook ${CMAKE_THREAD_LIBS_INIT} ${MPI_LIBRARIES})
target_link_libraries(mitosrun mitos)
target_link_libraries(mitosmpirun mitos mitosmpihook)
target_link_libraries(mitosprocess mitos symtabAPI)

# Install mitosrun, mitosprocess, and the libraries.
install(TARGETS mitos DESTINATION lib)
install(TARGETS mitosmpihook DESTINATION lib)
install(TARGETS mitosrun DESTINATION bin)
install(TARGETS mitosmpirun DESTINATION bin)
install(TARGETS mitosprocess DESTINATION bin)
