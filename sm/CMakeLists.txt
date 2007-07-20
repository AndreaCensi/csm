	INCLUDE_DIRECTORIES(lib)
	INCLUDE_DIRECTORIES(.)

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
		MESSAGE(SEND_ERROR "GSL not found.")
	ENDIF(GSL_FOUND)

	INCLUDE (FindCairo.cmake)                                                                                         

	IF(CAIRO_FOUND)
		MESSAGE(STATUS "Found Cairo at.")
		MESSAGE(STATUS "   PREFIX_CAIRO= ${PREFIX_CAIRO}")
		MESSAGE(STATUS "   CAIRO_CFLAGS = ${CAIRO_CFLAGS}")
		MESSAGE(STATUS "   CAIRO_INCLUDE_DIR = ${CAIRO_INCLUDE_DIR}")
		
		INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIR})	
	ELSE(CAIRO_FOUND)
		MESSAGE(WARNING " Cairo not found: plotting programs (e.g., 'log2pdf') will not be built.")
		MESSAGE(WARNING " install Cairo from http://cairographics.org ")
	ENDIF(CAIRO_FOUND)
	

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")

#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wall   -Wunreachable-code ")
	#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wmissing-prototypes  -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wnested-externs -fshort-enums -fno-common" )
	# -Wstrict-prototypes  -Wtraditional

#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unreachable-code")
	
#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
#	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

	# for realpath
	IF("${CMAKE_SYSTEM}" MATCHES "Linux")
	
		ADD_DEFINITIONS(-DLINUX)
	ENDIF("${CMAKE_SYSTEM}" MATCHES "Linux")

	########### Options ########## 
	SUBDIRS(lib/options)
	SUBDIRS(lib/json-c)
	SUBDIRS(lib/egsl)
	SUBDIRS(lib/gpc)
	SUBDIRS(csm)



########### SM ########## 



	########### SM applications ########## 
	
	MACRO(new_executable exec)
		ADD_EXECUTABLE(${exec} apps/${exec}.c)
		TARGET_LINK_LIBRARIES(${exec} csm-static ${GSL_LIBRARIES} gpc json-c egsl options)
		INSTALL(PROGRAMS ${exec} DESTINATION bin)
	ENDMACRO(new_executable exec)
	
	new_executable(sm0)
	new_executable(sm1)
	new_executable(sm2)
	new_executable(sm3)
	new_executable(json_extract)
	new_executable(json_split)
	new_executable(carmen2json)
	new_executable(ld_noise)
	new_executable(ld_slip)
	new_executable(ld_fisher)
	new_executable(ld_smooth)
	new_executable(ld_resample)
	new_executable(ld_remove_doubles)
	new_executable(ld_alternate)

	new_executable(ld_cluster_curv)
	new_executable(ld_linearize)
	
	new_executable(ld_stats)
	new_executable(ld_correct)

	new_executable(test_json_ld)
	new_executable(test_json)

	ADD_EXECUTABLE(test_math_utils csm/math_utils_test.c)
	TARGET_LINK_LIBRARIES(test_math_utils csm ${GSL_LIBRARIES})

	ENABLE_TESTING()
	ADD_TEST(test_math_utils test_math_utils)

########### Carmen2pdf ########## 
IF(CAIRO_FOUND)
	new_executable(carmen2pdf)
	new_executable(sm_animate)
	new_executable(log2pdf)
	
	
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CAIRO_CFLAGS}")
	TARGET_LINK_LIBRARIES(carmen2pdf -lcairo)
	TARGET_LINK_LIBRARIES(log2pdf  -lcairo)
	TARGET_LINK_LIBRARIES(sm_animate -lcairo)
ENDIF(CAIRO_FOUND)

SET(csm_sources 
./csm/clustering.c
./csm/gpm/gpm.c
./csm/icp/icp.c
./csm/icp/icp_corr_dumb.c
./csm/icp/icp_corr_tricks.c
./csm/icp/icp_covariance.c
./csm/icp/icp_loop.c
./csm/icp/icp_outliers.c
./csm/icp/icp_debug.c
./csm/json_journal.c
./csm/laser_data.c
./csm/laser_data_carmen.c
./csm/laser_data_fisher.c
./csm/laser_data_json.c
./csm/laser_data_load.c
./csm/laser_data_drawing.c
./csm/logging.c
./csm/math_utils.c
./csm/orientation.c
./csm/sm_options.c
./csm/utils.c
./lib/egsl/egsl.c
./lib/egsl/egsl_conversions.c
./lib/egsl/egsl_misc.c
./lib/egsl/egsl_ops.c
./lib/gpc/gpc.c
./lib/gpc/gpc_utils.c
./lib/json-c/arraylist.c
./lib/json-c/debug.c
./lib/json-c/JSON_checker.c
./lib/json-c/json_more_utils.c
./lib/json-c/json_object.c
./lib/json-c/json_tokener.c
./lib/json-c/json_util.c
./lib/json-c/linkhash.c
./lib/json-c/printbuf.c
./lib/options/options.c
./lib/options/options_interface.c
)

IF(CAIRO_FOUND)
	SET(csm_sources ${csm_sources} csm/laser_data_cairo.c)
ENDIF(CAIRO_FOUND)

ADD_LIBRARY(csm-static STATIC ${csm_sources})

TARGET_LINK_LIBRARIES(csm-static ${GSL_LIBRARIES})
#ADD_EXECUTABLE(test-static test.c)
#TARGET_LINK_LIBRARIES(test-static csm-static)

INSTALL(TARGETS csm-static
	ARCHIVE DESTINATION lib)
