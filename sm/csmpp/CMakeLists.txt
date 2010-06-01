cmake_minimum_required(VERSION 2.4)
PROJECT (CSMPP CXX)
SET(csmpp_version 0.1)

# Require we have pkgconfig installed
find_package(PkgConfig REQUIRED)
# Tell pkgconfig to look for CSM
pkg_check_modules(CSM REQUIRED csm)

# Look for CSM
IF(${CSM_FOUND})
	MESSAGE("CSM_LIBRARY_DIRS: ${CSM_LIBRARY_DIRS}")
	MESSAGE("CSM_LIBRARIES: ${CSM_LIBRARIES}")
	MESSAGE("CSM_INCLUDE_DIRS: ${CSM_INCLUDE_DIRS}")
	MESSAGE("CSM_LDFLAGS: " ${CSM_LDFLAGS})


	INCLUDE_DIRECTORIES(${CSM_INCLUDE_DIRS}) # important! 
	LINK_DIRECTORIES(${CSM_LIBRARY_DIRS})    # important! 
ELSE(${CSM_FOUND})	
	MESSAGE(FATAL_ERROR "CSM not found. Check that the environment variable PKG_CONFIG_PATH includes the path containing the file 'csm.pc'.")
ENDIF(${CSM_FOUND})
 
# Name of generated libraries
SET(dynamic_lib csmpp)
SET(static_lib csmpp-static)
SET(csmpp_sources csmpp.cpp)

SET(csmpp_c_flags "")
SET(csmpp_link_flags  ${CSM_LDFLAGS} )

# Creation of static library
ADD_LIBRARY(${static_lib} STATIC ${csmpp_sources})
TARGET_LINK_LIBRARIES(${static_lib} ${CSM_LIBRARIES})
INSTALL(TARGETS ${static_lib} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

# Creation of dynamic library
ADD_LIBRARY(${dynamic_lib} SHARED ${csmpp_sources})
TARGET_LINK_LIBRARIES(${dynamic_lib} ${CSM_LIBRARIES})
INSTALL(TARGETS ${dynamic_lib} LIBRARY DESTINATION lib)


# Installation of pkgconfig configuration file
CONFIGURE_FILE(csmpp.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/csmpp.pc @ONLY)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/csmpp.pc
	DESTINATION lib/pkgconfig)



