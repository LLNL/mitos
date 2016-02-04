# Include dirs required
include_directories(${MPI_INCLUDE_PATH})
include_directories(${DYNINST_INCLUDE_DIR})

# Source and header files
set(MITOS_SRC_FILES procsmpl.cpp mmap_processor.cpp mattr.cpp mitosoutput.cpp Mitos.cpp)
set(MITOS_HDR_FILES procsmpl.h mmap_processor.h mattr.h Mitos.h)
set(MITOS_HDR_PUBLIC Mitos.h)
set(MITOSHOOKS_SRC_FILES mitoshooks.cpp)

# Libraries (created)
add_library(mitos SHARED ${MITOS_SRC_FILES})
add_library(mitoshooks SHARED ${MITOSHOOKS_SRC_FILES})

# Binaries
add_executable(mitosrun mitosrun.cpp)

# Linkage
target_link_libraries(mitos mitoshooks hwloc ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(mitosrun mitos)

if(MPI_FOUND)
    target_link_libraries(mitos ${MPI_LIBRARIES}) 
endif()

if(Dyninst_FOUND)
    target_link_libraries(mitos symtabAPI instructionAPI parseAPI) 
endif()


# Install
install(FILES ${MITOS_HDR_PUBLIC}
        DESTINATION "include/Mitos")
install(TARGETS mitosrun 
        EXPORT mitos-targets
        DESTINATION bin)
install(TARGETS mitos mitoshooks
        EXPORT mitos-targets
        DESTINATION lib)

