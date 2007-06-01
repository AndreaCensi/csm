#PROJECT (scan_matching_lib C)

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

	IF(CAIRO_FOUND)
		MESSAGE(STATUS "Found Cairo at.")
		MESSAGE(STATUS "   PREFIX_CAIRO= ${PREFIX_CAIRO}")
		MESSAGE(STATUS "   CAIRO_CFLAGS = ${CAIRO_CFLAGS}")
		MESSAGE(STATUS "   CAIRO_INCLUDE_DIR = ${CAIRO_INCLUDE_DIR}")
		
		INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIR})	
	ELSE(CAIRO_FOUND)
		MESSAGE(WARNING " Cairo not found: carmen2pdf will not be built.")
		MESSAGE(WARNING " install Cairo from http://cairographics.org ")
	ENDIF(CAIRO_FOUND)
	


	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wall  -Wconversion  -Wunreachable-code ")
#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing")

	
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unreachable-code")
	

	IF(APPLE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-double")
		MESSAGE(STATUS "Using optimizations: ")
#		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450 -O3")
#		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -fomit-frame-pointer -mtune=7450 -mcpu=7450 -O3")
	ELSE(APPLE)
		# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
		# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -fomit-frame-pointer")

		#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=pentium3 -msse -O3 -pipe -fomit-frame-pointer")

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
	SUBDIRS(lib/options)
	SUBDIRS(lib/json-c)

########### SM ########## 

	ADD_LIBRARY(sm STATIC 
		src/journal.c
		src/laser_data.c
		src/laser_data_fisher.c
		src/laser_data_json.c
		src/laser_data_carmen.c
		src/json_journal.c
		src/math_utils.c
		src/utils.c
		src/orientation.c
		src/clustering.c
		src/logging.c
		src/sm_options.c
			
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

	INCLUDE_DIRECTORIES(lib/egsl)
	INCLUDE_DIRECTORIES(lib/gpc)
	INCLUDE_DIRECTORIES(lib/json-c)
	INCLUDE_DIRECTORIES(lib)

	
	TARGET_LINK_LIBRARIES(sm json-c)


	TARGET_LINK_LIBRARIES(sm ${GSL_LIBRARIES})
	INSTALL(TARGETS sm ARCHIVE DESTINATION lib)
	INSTALL(FILES src/sm.h  DESTINATION include)
	INSTALL(FILES src/laser_data.h  DESTINATION include)

	########### SM applications ########## 

		ADD_EXECUTABLE(sm0 apps/sm0.c)
		TARGET_LINK_LIBRARIES(sm0 sm ${GSL_LIBRARIES})

		ADD_EXECUTABLE(sm1 apps/sm1.c)
		TARGET_LINK_LIBRARIES(sm1 sm ${GSL_LIBRARIES} options)
	
		ADD_EXECUTABLE(sm2 apps/sm2.c)
		TARGET_LINK_LIBRARIES(sm2 sm ${GSL_LIBRARIES} options)

		ADD_EXECUTABLE(ld_fisher apps/ld_fisher.c)
		TARGET_LINK_LIBRARIES(ld_fisher  sm ${GSL_LIBRARIES} options)

		ADD_EXECUTABLE(test_math_utils src/math_utils_test.c)
		TARGET_LINK_LIBRARIES(test_math_utils sm ${GSL_LIBRARIES})

		ADD_EXECUTABLE(json_pipe apps/json_pipe.c)
		TARGET_LINK_LIBRARIES(json_pipe sm ${GSL_LIBRARIES} options)

		ADD_EXECUTABLE(json_extract apps/json_extract.c)
		TARGET_LINK_LIBRARIES(json_extract json-c options)

		ADD_EXECUTABLE(test_json_ld apps/test_json_ld.c)
		TARGET_LINK_LIBRARIES(test_json_ld sm ${GSL_LIBRARIES})

		ADD_EXECUTABLE(test_json apps/test_json.c)
		TARGET_LINK_LIBRARIES(test_json sm ${GSL_LIBRARIES})

		ADD_EXECUTABLE(carmen2json apps/carmen2json.c)
		TARGET_LINK_LIBRARIES(carmen2json sm)

		ADD_EXECUTABLE(ld_noise apps/ld_noise.c)
		TARGET_LINK_LIBRARIES(ld_noise sm ${GSL_LIBRARIES} options)

		ADD_EXECUTABLE(ld_slip apps/ld_slip.c)
		TARGET_LINK_LIBRARIES(ld_slip sm ${GSL_LIBRARIES} options)

	########### Carmen2pdf ########## 
IF(CAIRO_FOUND)
	ADD_EXECUTABLE(carmen2pdf apps/carmen2pdf.c)
	TARGET_LINK_LIBRARIES(carmen2pdf sm ${GSL_LIBRARIES})
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I../deploy/include/cairo")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CAIRO_CFLAGS}")
	TARGET_LINK_LIBRARIES(carmen2pdf options -lcairo)

ENDIF(CAIRO_FOUND)
