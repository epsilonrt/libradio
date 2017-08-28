###############################################################################
# Copyright © 2017 epsilonRT, All rights reserved.                            #
# This software is governed by the CeCILL license <http://www.cecill.info>    #
###############################################################################
# radio CMakeLists.txt

# set packaging dir
if(NOT CPACK_PACKAGE_DIRECTORY)
  set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
endif()

file(GLOB src_radio ${RADIO_SRC_DIR}/*.c)
file(GLOB src_radio_cpp ${RADIO_SRC_DIR}/*.cpp)
file(GLOB hdr_radio ${RADIO_INC_DIR}/radio/*.h)
file(GLOB hdr_radio_cpp ${RADIO_INC_DIR}/radio/*.hpp)


configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

set(hdr_public ${hdr_radio} ${hdr_radio_cpp} ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set (libsrc ${src_radio} ${src_radio_cpp})

include_directories(
  ${PROJECT_SOURCE_DIR}/3rdparty/ssdv/include
  ${SYSIO_INC_DIR} 
  ${RADIO_INC_DIR} 
  ${RADIO_SRC_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

add_library(radio-shared SHARED ${libsrc})
target_compile_definitions(radio-shared PUBLIC LOG_ASSERT SYSIO_WITH_CONFIG_H)
set_target_properties(radio-shared PROPERTIES 
  OUTPUT_NAME radio 
  CLEAN_DIRECT_OUTPUT 1 
  C_STANDARD 99 
  VERSION ${SYSIO_VERSION} 
  SOVERSION ${SYSIO_VERSION_SHORT}
  PUBLIC_HEADER "${hdr_public}"
  )

if(SYSIO_WITH_STATIC)
  add_library(radio-static STATIC ${libsrc})
  target_compile_definitions(radio-static PUBLIC LOG_ASSERT SYSIO_WITH_CONFIG_H)
  set_target_properties(radio-static PROPERTIES
    OUTPUT_NAME radio 
    CLEAN_DIRECT_OUTPUT 1 
  C_STANDARD 99 
    VERSION ${SYSIO_VERSION} 
  )
endif(SYSIO_WITH_STATIC)

# CMake Package ----------------------------------------------------------------
# The interesting stuff goes here
# ===============================

# Add all targets to the build-tree export set
if (SYSIO_WITH_STATIC)
  set (exported_targets radio-shared radio-static)
else(SYSIO_WITH_STATIC)
  set (exported_targets radio-shared)
endif(SYSIO_WITH_STATIC)

export(TARGETS ${exported_targets}
  FILE "${PROJECT_BINARY_DIR}/radio.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE radio)

# Create the radio-config.cmake and radio-config-version files
file(RELATIVE_PATH REL_LIB_DIR "${RADIO_INSTALL_CMAKE_DIR}" 
  "${INSTALL_LIB_DIR}")
file(RELATIVE_PATH REL_INCLUDE_DIR "${RADIO_INSTALL_CMAKE_DIR}"
  "${INSTALL_INCLUDE_DIR}")

# ... for the build tree
set(CONF_INCLUDE_DIRS "${SYSIO_INC_DIR}" "${RADIO_INC_DIR}" "${RADIO_SRC_DIR}" "${PROJECT_BINARY_DIR}")
set(CONF_LIB_DIRS "${PROJECT_BINARY_DIR}/lib/radio")
configure_file(radio-config.cmake.in
  "${PROJECT_BINARY_DIR}/radio-config.cmake" @ONLY)
   

# ... for the install tree
set(CONF_INCLUDE_DIRS "\${RADIO_CMAKE_DIR}/${REL_INCLUDE_DIR}")
set(CONF_LIB_DIRS "\${RADIO_CMAKE_DIR}/${REL_LIB_DIR}")
configure_file(radio-config.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/radio-config.cmake" @ONLY)

# ... for both
configure_file(radio-config-version.cmake.in
  "${PROJECT_BINARY_DIR}/radio-config-version.cmake" @ONLY)

# Install the radio-config.cmake and radio-config-version.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/radio-config.cmake"
  "${PROJECT_BINARY_DIR}/radio-config-version.cmake"
  DESTINATION "${RADIO_INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT radio DESTINATION
  "${RADIO_INSTALL_CMAKE_DIR}" COMPONENT dev)

install (TARGETS radio-shared
  # IMPORTANT: Add the sysio++ library to the "export-set"
  EXPORT radio
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/radio" COMPONENT dev
  )

if(SYSIO_WITH_STATIC)
  install (TARGETS radio-static 
    EXPORT radio
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  )
endif(SYSIO_WITH_STATIC)


## pkg-config ------------------------------------------------------------------

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/radio.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/radio.pc @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/radio.pc 
  DESTINATION "${INSTALL_LIB_DIR}/pkgconfig"
  COMPONENT dev)
