PROJECT (scan_matching_lib C)

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

ADD_LIBRARY(sm STATIC 
	src/journal.c
	src/laser_data.c
	src/math_utils.c
	src/orientation.c
	src/clustering.c
	
	src/icp/icp.c
	src/icp/icp_loop.c
	src/icp/icp_corr_dumb.c
	src/icp/icp_corr_tricks.c
	src/icp/icp_outliers.c
	src/icp/icp_covariance.c
	
	lib/gpc/gpc.c
	lib/gpc/gpc_utils.c
)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ilib/gpc")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ilib/egsl")


########### Easy GSL ########## 
ADD_LIBRARY(egsl STATIC lib/egsl/egsl.c lib/egsl/egsl_ops.c)

ADD_EXECUTABLE(test_egsl lib/egsl/egsl_test.c)
TARGET_LINK_LIBRARIES(test_egsl egsl  ${GSL_LIBRARIES})


TARGET_LINK_LIBRARIES(sm egsl ${GSL_LIBRARIES})


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wall -Wno-long-double -W -Wconversion  -Wunreachable-code ")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")

IF(APPLE)
#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450 -O3")
ELSE(APPLE)

ENDIF(APPLE)

INSTALL(TARGETS sm ARCHIVE DESTINATION lib)
INSTALL(FILES src/sm.h  DESTINATION include)


ADD_EXECUTABLE(sm0 apps/sm0.c)
TARGET_LINK_LIBRARIES(sm0 sm ${GSL_LIBRARIES})

ADD_EXECUTABLE(test_math_utils src/math_utils_test.c)
TARGET_LINK_LIBRARIES(test_math_utils sm ${GSL_LIBRARIES})
