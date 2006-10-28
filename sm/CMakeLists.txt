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

	INCLUDE (FindCairo.cmake)                                                                                         
SET(CAIRO_FOUND no)

	IF(CAIRO_FOUND)
		MESSAGE(STATUS "Found Cairo at.")
		MESSAGE(STATUS "   PREFIX_CAIRO= ${PREFIX_CAIRO}")
		MESSAGE(STATUS "   CAIRO_CFLAGS = ${CAIRO_CFLAGS}")
		
#		INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})	
#		LINK_DIRECTORIES(${GSL_LINK_DIRECTORIES})		 
	ELSE(CAIRO_FOUND)
		MESSAGE(WARNING "Cairo not found: carmen2pdf will not be built.")
	ENDIF(CAIRO_FOUND)
	


	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wall -W -Wconversion  -Wunreachable-code ")
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")

	IF(APPLE)
		MESSAGE(STATUS "Using optimizations: ")
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-double")
#		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450 -O3")
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -fomit-frame-pointer -mtune=7450 -mcpu=7450 -O3")
	ELSE(APPLE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

	ENDIF(APPLE)


	########### Easy GSL ########## 
		ADD_LIBRARY(egsl STATIC 
			lib/egsl/egsl.c 
			lib/egsl/egsl_ops.c
			lib/egsl/egsl_conversions.c
			lib/egsl/egsl_misc.c
		)

		ADD_EXECUTABLE(test_egsl lib/egsl/egsl_test.c)
		TARGET_LINK_LIBRARIES(test_egsl egsl  ${GSL_LIBRARIES})

		ADD_EXECUTABLE(egsl_test_allocation lib/egsl/egsl_test_allocation.c)
		TARGET_LINK_LIBRARIES(egsl_test_allocation egsl  ${GSL_LIBRARIES})

	########### Options ########## 
		ADD_LIBRARY(options STATIC lib/options/options.c)

########### SM ########## 

	ADD_LIBRARY(sm STATIC 
		src/journal.c
		src/laser_data.c
		src/laser_data_fisher.c
		src/math_utils.c
		src/orientation.c
		src/clustering.c
	
		src/icp/icp.c
		src/icp/icp_loop.c
		src/icp/icp_corr_dumb.c
		src/icp/icp_corr_tricks.c
		src/icp/icp_outliers.c
		src/icp/icp_covariance.c
	
		src/gpm/gpm.c
	
		lib/gpc/gpc.c
		lib/gpc/gpc_utils.c

		lib/egsl/egsl.c 
		lib/egsl/egsl_ops.c
		lib/egsl/egsl_conversions.c
		lib/egsl/egsl_misc.c
	)

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ilib/gpc")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ilib/egsl")

	TARGET_LINK_LIBRARIES(sm ${GSL_LIBRARIES})
	INSTALL(TARGETS sm ARCHIVE DESTINATION lib)
	INSTALL(FILES src/sm.h  DESTINATION include)

	########### SM applications ########## 

		ADD_EXECUTABLE(sm0 apps/sm0.c)
		TARGET_LINK_LIBRARIES(sm0 sm ${GSL_LIBRARIES})

		ADD_EXECUTABLE(test_math_utils src/math_utils_test.c)
		TARGET_LINK_LIBRARIES(test_math_utils sm ${GSL_LIBRARIES})

	########### Carmen2pdf ########## 
IF(CAIRO_FOUND)
	ADD_EXECUTABLE(carmen2pdf apps/carmen2pdf.c)
	TARGET_LINK_LIBRARIES(carmen2pdf sm ${GSL_LIBRARIES})
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I../deploy/include/cairo")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CAIRO_CFLAGS}")
	TARGET_LINK_LIBRARIES(carmen2pdf options -lcairo)
ENDIF(CAIRO_FOUND)
