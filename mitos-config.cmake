get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/mitos-targets.cmake)
get_filename_component(Mitos_INCLUDE_DIRS
   "${SELF_DIR}/../../include/Mitos" ABSOLUTE)
