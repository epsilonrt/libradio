###############################################################################
# Copyright © 2017 epsilonRT, All rights reserved.                            #
# This software is governed by the CeCILL license <http://www.cecill.info>    #
###############################################################################
# radio top-level CMakeLists.txt

set (RADIO_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set (RADIO_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set (RADIO_INSTALL_CMAKE_DIR "${INSTALL_CMAKE_DIR}/radio/cmake")

add_subdirectory (lib)
