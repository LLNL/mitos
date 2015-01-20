include(CheckIncludeFiles)
include(CheckCSourceCompiles)

#
# ensure perf_paranoid file is present.
#
set(PERF_PARANOID /proc/sys/kernel/perf_event_paranoid)
if(EXISTS ${PERF_PARANOID})
  set(HAVE_PERF_PARANOID TRUE)
else()
  message(ERROR "${PERF_PARANOID} not found, perfsmpl requires a kernel with perf_events enabled!")
  set(HAVE_PERF_PARANOID FALSE)
endif()

#
# linux/perf_event.h header.
#
check_include_files(linux/perf_event.h HAVE_PERF_EVENT_H)

#
# Ensure we can compile with the perf_events symbols used for memory sampling.
#
check_c_source_compiles("
// This program will compile if a version of linux perf_events is
// available that supports memory sampling.
#include <stdio.h>
#include <linux/perf_event.h>

int main(int argc, char **argv) {
  printf(\"PERF_SAMPLE_WEIGHT is %d\", PERF_SAMPLE_WEIGHT);
  printf(\"PERF_SAMPLE_DATA_SRC is %d\", PERF_SAMPLE_DATA_SRC);
  printf(\"PERF_COUNT_SW_DUMMY is %d\", PERF_COUNT_SW_DUMMY);
}"
HAVE_PERF_EVENT_MEM_SAMPLING)

# Make sure all these checks are true and set PerfEvents_FOUND accordingly.
find_package_handle_standard_args(PerfEvents
  FAIL_MESSAGE "ERROR: Need a kernel with perf_events support for memory sampling."
  REQUIRED_VARS HAVE_PERF_PARANOID HAVE_PERF_EVENT_H HAVE_PERF_EVENT_MEM_SAMPLING)

