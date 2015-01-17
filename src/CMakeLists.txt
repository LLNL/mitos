project(perfsmpl)
cmake_minimum_required(VERSION 2.8)

# perf_event
set(PERF_PARANOID /proc/sys/kernel/perf_event_paranoid)
if(EXISTS ${PERF_PARANOID})
    message(STATUS "${PERF_PARANOID} found")
else()
    message(FATAL_ERROR "${PERF_PARANOID} not found, perfsmpl requires a kernel with perf_events enabled!")
endif()

# pthreads
find_package(Threads)

set(SRC_FILES perfsmpl.cpp mattr.cpp)
set(HDR_FILES perfsmpl.h mattr.h SAPI.h)

add_library(perfsmpl SHARED ${SRC_FILES})
add_library(perfsmpl_static STATIC ${SRC_FILES})

add_executable(test test.cpp)
add_executable(memaxes memaxes.cpp)

target_link_libraries(perfsmpl ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test perfsmpl symtabAPI)
target_link_libraries(memaxes perfsmpl symtabAPI)
