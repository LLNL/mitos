# Include dirs required
include_directories(${DYNINST_INCLUDE_DIR})

# Source and header files
set(MITOS_SRC_FILES perfsmpl.cpp mattr.cpp mitosoutput.cpp Mitos.cpp)
set(MITOS_HDR_FILES perfsmpl.h mattr.h Mitos.h)
set(MITOS_HDR_PUBLIC Mitos.h)

# Libraries (created)
add_library(mitos SHARED ${MITOS_SRC_FILES})

# Binaries
add_executable(mitosrun mitosrun.cpp)

# Linkage
target_link_libraries(mitos hwloc symtabAPI instructionAPI parseAPI ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(mitosrun mitos)

# Install
install(FILES ${MITOS_HDR_PUBLIC}
        DESTINATION include)
install(TARGETS mitosrun 
        EXPORT mitos-targets
        DESTINATION bin)
install(TARGETS mitos 
        EXPORT mitos-targets
        DESTINATION lib)

