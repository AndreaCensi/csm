cmake_minimum_required(VERSION 2.4)

project(raytracer)


find_package(PkgConfig REQUIRED)
pkg_check_modules(CSM REQUIRED csm)

IF(${CSM_FOUND})
	MESSAGE("CSM_LIBRARY_DIRS: ${CSM_LIBRARY_DIRS}")
	MESSAGE("CSM_LIBRARIES: ${CSM_LIBRARIES}")
	MESSAGE("CSM_INCLUDE_DIRS: ${CSM_INCLUDE_DIRS}")

	INCLUDE_DIRECTORIES(${CSM_INCLUDE_DIRS})
	LINK_DIRECTORIES(${CSM_LIBRARY_DIRS})

ELSE(${CSM_FOUND})	
	MESSAGE(FATAL_ERROR "CSM not found. Check that the environment variable PKG_CONFIG_PATH includes the path containing the file 'csm.pc'.")
ENDIF(${CSM_FOUND})		


set(raytracer_SRCS
	raytracer.cpp
	simplemap.cpp
)


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall")

add_executable(raytracer ${raytracer_SRCS})

target_link_libraries(raytracer ${CSM_LIBRARIES})

