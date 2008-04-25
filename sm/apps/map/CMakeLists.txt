project(map)


find_package(PkgConfig REQUIRED)
pkg_check_modules(CSM REQUIRED csm)

IF(${CSM_FOUND})
	MESSAGE("CSM_LIBRARY_DIRS: ${CSM_LIBRARY_DIRS}")
	MESSAGE("CSM_LIBRARIES: ${CSM_LIBRARIES}")
	MESSAGE("CSM_INCLUDE_DIRS: ${CSM_INCLUDE_DIRS}")

	INCLUDE_DIRECTORIES(${CSM_INCLUDE_DIRS})
	LINK_DIRECTORIES(${CSM_LIBRARY_DIRS})

ELSE(${CSM_FOUND})	
	MESSAGE(SEND_ERROR, "CSM not found.")
ENDIF(${CSM_FOUND})		



# the variable "qtproject_SRCS" contains all .cpp files of this project
set(map1_SRCS
	map1.cpp
)


add_executable(map1 ${map1_SRCS})

target_link_libraries(map1 ${CSM_LIBRARIES})

