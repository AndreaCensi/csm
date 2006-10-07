PROJECT (icp C)

	INCLUDE (FindGSL.cmake)                                                                                         

	IF(GSL_FOUND)
		MESSAGE(STATUS "Found GSL:")
		MESSAGE(STATUS "   GSL_LIBRARIES = ${GSL_LIBRARIES}")
		MESSAGE(STATUS "   GSL_INCLUDE_DIR = ${GSL_INCLUDE_DIR}")
		MESSAGE(STATUS "   CMAKE_GSL_CXX_FLAGS = ${CMAKE_GSL_CXX_FLAGS}")
		MESSAGE(STATUS "   GSL_LINK_DIRECTORIES = ${GSL_LINK_DIRECTORIES}")
		
		INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})	
		LINK_DIRECTORIES(${GSL_LINK_DIRECTORIES})		 
	ELSE(GSL_FOUND)
		MESSAGE(ERROR "GSL not found.")
	ENDIF(GSL_FOUND)

ADD_LIBRARY(sm STATIC icp.c icp_correspondences_dumb.c 
icp_correspondences_tricks.c icp_loop.c journal.c laser_data.c math_utils.c
icp_outliers.c gsl_stack.c orientation.c clustering.c gpm.c 
lib/gpc/gpc.c
lib/gpc/gpc_utils.c
)

TARGET_LINK_LIBRARIES(sm ${GSL_LIBRARIES})


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -W -Wconversion  -Wunreachable-code ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ilib/gpc")


INSTALL(TARGETS sm ARCHIVE DESTINATION lib)
INSTALL(FILES sm.h  DESTINATION include)


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=powerpc -O3")

ADD_EXECUTABLE(sm_test sm_test.c)
TARGET_LINK_LIBRARIES(sm_test sm ${GSL_LIBRARIES})
