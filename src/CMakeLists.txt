set(SRC_FILES perfsmpl.cpp mattr.cpp Mitos.cpp)
set(HDR_FILES perfsmpl.h mattr.h Mitos.h)

# Add shared and static libs
add_library(perfsmpl SHARED ${SRC_FILES})
add_library(perfsmpl_static STATIC ${SRC_FILES})

# Static lib has same root name as dynamic lib
set_target_properties(perfsmpl_static PROPERTIES OUTPUT_NAME perfsmpl)

add_executable(test test.cpp)
add_executable(mitosrun mitosrun.cpp)
add_executable(mitosprocess mitosprocess.cpp)

target_link_libraries(perfsmpl ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test perfsmpl symtabAPI)
target_link_libraries(mitosrun perfsmpl)
target_link_libraries(mitosprocess perfsmpl symtabAPI)

include_directories(${DYNINST_INCLUDE_DIR})

# Install paspirun and the libraries.
install(TARGETS mitosrun DESTINATION bin)
install(TARGETS mitosprocess DESTINATION bin)
install(TARGETS perfsmpl perfsmpl_static DESTINATION lib)
