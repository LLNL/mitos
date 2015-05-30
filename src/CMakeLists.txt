# Include dirs required
include_directories(${MPI_INCLUDE_PATH})
include_directories(${DYNINST_INCLUDE_DIR})

# Source and header files
set(MITOS_SRC_FILES perfsmpl.cpp mattr.cpp mitosoutput.cpp Mitos.cpp)
set(MITOS_HDR_FILES perfsmpl.h mattr.h Mitos.h)
set(MITOS_HDR_PUBLIC Mitos.h)
set(MITOS_MPIHOOK_SRC_FILES mitosmpihook.cpp)

# Libraries (created)
add_library(mitos SHARED ${MITOS_SRC_FILES})
add_library(mitosmpihook SHARED ${MITOS_SRC_FILES} ${MITOS_MPIHOOK_SRC_FILES})

# Binaries
add_executable(mitosrun mitosrun.cpp)
add_executable(mitosmpirun mitosmpirun.cpp)

# Linkage
target_link_libraries(mitos hwloc symtabAPI instructionAPI parseAPI ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(mitosmpihook ${CMAKE_THREAD_LIBS_INIT} ${MPI_LIBRARIES})
target_link_libraries(mitosrun mitos)
target_link_libraries(mitosmpirun mitos mitosmpihook)

# Install
install(FILES ${MITOS_HDR_PUBLIC}
        DESTINATION include/Mitos)
install(TARGETS mitosrun mitosmpirun 
        EXPORT mitos-targets
        DESTINATION bin)
install(TARGETS mitos mitosmpihook
        EXPORT mitos-targets
        DESTINATION lib/Mitos)

