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

ADD_LIBRARY(icp STATIC icp.c icp_correspondences_dumb.c 
icp_correspondences_tricks.c icp_loop.c journal.c laser_data.c math_utils.c
icp_outliers.c gsl_stack.c orientation.c clustering.c gpm.c
)

TARGET_LINK_LIBRARIES(icp ${GSL_LIBRARIES})

ADD_EXECUTABLE(test test.c)
#TARGET_LINK_LIBRARIES(gpc_test gpc ${GSL_LIBRARIES})

#INSTALL(TARGETS icp LIBRARY DESTINATION lib)
INSTALL(TARGETS icp ARCHIVE DESTINATION lib)

INSTALL(FILES icp.h        DESTINATION include)
INSTALL(FILES laser_data.h DESTINATION include)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-double -Wall -W -Wmissing-prototypes -Wconversion  -Wunreachable-code ")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
