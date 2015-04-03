project(Mitos C CXX)
cmake_minimum_required(VERSION 2.8)

# Version info
set(MITOS_MAJOR_VERSION 0)
set(MITOS_MINOR_VERSION 8)
set(MITOS_PATCH_VERSION 8)
set(MITOS_VERSION ${MITOS_MAJOR_VERSION}.${MITOS_MINOR_VERSION}.${MITOS_PATCH_VERSION})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH 
    "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH 
    "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH 
    "Installation directory for header files")
set(DEF_INSTALL_CMAKE_DIR lib/CMake/Mitos)
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH 
    "Installation directory for CMake files")
 
# Make relative paths absolute
foreach(p LIB BIN INCLUDE CMAKE)
    set(var INSTALL_${p}_DIR)
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

# Include project-local cmake files.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# Subdirectories
add_subdirectory(src)

# Add all targets
export(TARGETS mitos mitosmpihook mitosrun mitosmpirun 
  FILE "${PROJECT_BINARY_DIR}/MitosTargets.cmake")
 
# Export the package
export(PACKAGE Mitos)
 
# Create the MitosConfig.cmake and MitosConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(MitosConfig.cmake.in
  "${PROJECT_BINARY_DIR}/MitosConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${MITOS_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(MitosConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/MitosConfig.cmake" @ONLY)
# ... for both
configure_file(MitosConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/MitosConfigVersion.cmake" @ONLY)
 
# Install the MitosConfig.cmake and MitosConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/MitosConfig.cmake"
  "${PROJECT_BINARY_DIR}/MitosConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
install(EXPORT MitosTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)
