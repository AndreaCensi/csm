cmake_minimum_required(VERSION 2.4)
project(myproject)

# Require we have pkgconfig installed
find_package(PkgConfig REQUIRED)
# Tell pkgconfig to look for CSM
pkg_check_modules(CSM REQUIRED csm)

IF(${CSM_FOUND})
	MESSAGE("CSM_LIBRARY_DIRS: ${CSM_LIBRARY_DIRS}")
	MESSAGE("CSM_LIBRARIES: ${CSM_LIBRARIES}")
	MESSAGE("CSM_INCLUDE_DIRS: ${CSM_INCLUDE_DIRS}")

	INCLUDE_DIRECTORIES(${CSM_INCLUDE_DIRS}) # important! 
	LINK_DIRECTORIES(${CSM_LIBRARY_DIRS})    # important! 
ELSE(${CSM_FOUND})	
	MESSAGE(FATAL_ERROR "CSM not found. Check that the environment variable PKG_CONFIG_PATH includes the path containing the file 'csm.pc'.")
ENDIF(${CSM_FOUND})		

add_executable(myprogram myprogram.c)

target_link_libraries(myprogram ${CSM_LIBRARIES}) # important! 

