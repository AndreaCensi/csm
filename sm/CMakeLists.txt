PROJECT (CSM C CXX)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

cmake_minimum_required(VERSION 2.4)
set(csm_link_flags "-L.")

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

FIND_PACKAGE(GSL REQUIRED)
IF(GSL_FOUND)
	MESSAGE(STATUS "Found GSL:")
	MESSAGE(STATUS "   GSL_LIBRARIES = ${GSL_LIBRARIES}")
	MESSAGE(STATUS "   GSL_INCLUDE_DIR = ${GSL_INCLUDE_DIR}")
	MESSAGE(STATUS "   CMAKE_GSL_CXX_FLAGS = ${CMAKE_GSL_CXX_FLAGS}")
	MESSAGE(STATUS "   GSL_LINK_DIRECTORIES = '${GSL_LINK_DIRECTORIES}'")

 	
	set(csm_c_flags "${csm_c_flags} -I${GSL_INCLUDE_DIR}")
	if(GSL_LINK_DIRECTORIES)	
#		set(csm_link_flags "${csm_link_flags}
		set(csm_link_flags "${csm_link_flags} -L${GSL_LINK_DIRECTORIES}")
	endif(GSL_LINK_DIRECTORIES)	

	set(csm_link_flags "${csm_link_flags} ${GSL_LIBRARIES}")
	
ELSE(GSL_FOUND)
	MESSAGE(FATAL_ERROR "GSL not found.")
ENDIF(GSL_FOUND)

FIND_PACKAGE(Cairo)
IF(CAIRO_FOUND)
	MESSAGE(STATUS "Found Cairo at.")
	MESSAGE(STATUS "   PREFIX_CAIRO= ${PREFIX_CAIRO}")
	MESSAGE(STATUS "   CAIRO_CFLAGS = ${CAIRO_CFLAGS}")
	MESSAGE(STATUS "   CAIRO_LIBRARY = ${CAIRO_LIBRARY}")
	MESSAGE(STATUS "   CAIRO_INCLUDE_DIR = ${CAIRO_INCLUDE_DIR}")
	
	set(csm_link_flags "${csm_link_flags} ${CAIRO_LIBRARY}")
	SET(csm_c_flags "${csm_c_flags} ${CAIRO_CFLAGS} -I${CAIRO_INCLUDE_DIR}")
	
ELSE(CAIRO_FOUND)
	MESSAGE(WARNING "* Cairo not found: plotting programs (e.g., 'log2pdf') will not be built.")
	MESSAGE(WARNING "* install Cairo from http://cairographics.org ")
ENDIF(CAIRO_FOUND)





MESSAGE(STATUS "csm_c_flags = ${csm_c_flags}")
MESSAGE(STATUS "csm_link_flags = ${csm_link_flags}")


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${csm_c_flags}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -ggdb -Wall")


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





########### SM applications ########## 

MACRO(new_executable exec)
	ADD_EXECUTABLE(${exec} apps/${exec}.c)
	TARGET_LINK_LIBRARIES(${exec} csm-static ${csm_link_flags})
	INSTALL(TARGETS ${exec} DESTINATION bin)
ENDMACRO(new_executable exec)

# new_executable(sm0)
new_executable(sm1)
new_executable(sm2)
new_executable(sm3)
new_executable(json_extract)
new_executable(json_extract_field)
new_executable(json_decimate)
new_executable(json_split)
new_executable(json_pipe)
new_executable(carmen2json)
new_executable(json2carmen)
new_executable(ld_noise)
new_executable(ld_slip)
new_executable(ld_fisher)
new_executable(ld_smooth)
new_executable(ld_resample)
new_executable(ld_remove_doubles)
new_executable(ld_alternate)
new_executable(ld_recover)

new_executable(ld_purify)
new_executable(ld_select)
new_executable(ld_cluster_curv)
new_executable(ld_linearize)

new_executable(json2matlab)

new_executable(ld_exp_tro1)

# this are some random tests
# new_executable(ld_stats)
# new_executable(ld_correct)

new_executable(test_json_ld)
new_executable(test_json)



IF(CAIRO_FOUND)
	new_executable(sm_animate)
	new_executable(log2pdf)
ENDIF(CAIRO_FOUND)


ADD_EXECUTABLE(test_math_utils_sanity csm/test_math_utils_sanity.c)
TARGET_LINK_LIBRARIES(test_math_utils_sanity csm-static ${GSL_LIBRARIES})

ENABLE_TESTING()
ADD_TEST(test_math_utils_sanity test_math_utils_sanity)


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
./csm/mbicp/mbicp_interface.c      
./csm/mbicp/MbICP.c      
./csm/mbicp/calcul.c     
./csm/mbicp/percolate.c  
./csm/mbicp/sp_matrix.c
./csm/hsm/hsm.c
./csm/hsm/hsm_interface.c
./csm/json_journal.c
./csm/laser_data.c
./csm/laser_data_carmen.c
./csm/laser_data_fisher.c
./csm/laser_data_json.c
./csm/laser_data_load.c
./csm/laser_data_drawing.c
./csm/laser_data_bbox.c
./csm/logging.c
./csm/math_utils.c
./csm/math_utils_gsl.c
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
TARGET_LINK_LIBRARIES(csm-static ${csm_link_flags})
INSTALL(TARGETS csm-static ARCHIVE DESTINATION lib)

ADD_LIBRARY(csm SHARED ${csm_sources})
TARGET_LINK_LIBRARIES(csm ${csm_link_flags})
INSTALL(TARGETS csm ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)


IF(COMPILE_HSM)

INCLUDE_DIRECTORIES(/sw/include)
LINK_DIRECTORIES(/sw/lib)

ADD_EXECUTABLE(hsm_test00 ./csm/hsm/hsm_test00.c ./csm/hsm/hsm.c)
#TARGET_LINK_LIBRARIES(hsm_test00 csm-static ${csm_link_flags} pgm pnm pbm)
	TARGET_LINK_LIBRARIES(hsm_test00 csm-static ${csm_link_flags} netpbm)
INSTALL(PROGRAMS hsm_test00 DESTINATION bin)

ENDIF(COMPILE_HSM)


SUBDIRS(pkg-config)

if(0)
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${csm_c_flags} -Wall")

FILE(GLOB_RECURSE csm_headers csm/*.h)
FILE(GLOB_RECURSE lib_headers lib/*.h)

foreach(header ${csm_headers} ${lib_headers})
	GET_FILENAME_COMPONENT(header_base ${header} NAME_WE)
	GET_FILENAME_COMPONENT(header_dir ${header} PATH)

#	MESSAGE("Ciao ${header_base} ${header_dir}")
	
	CONFIGURE_FILE(header_test.c.in ${header_dir}/${header_base}_header_test.cpp @ONLY)	
	ADD_EXECUTABLE(${header_base}_header_test ${header_dir}/${header_base}_header_test.cpp)
	SET_TARGET_PROPERTIES(${header_base}_header_test  PROPERTIES LINKER_LANGUAGE CXX)
	TARGET_LINK_LIBRARIES(${header_base}_header_test  csm-static ${csm_link_flags})

	CONFIGURE_FILE(header_test.c.in ${header_dir}/${header_base}_header_test2.c @ONLY)	
	ADD_EXECUTABLE(${header_base}_header_test2 ${header_dir}/${header_base}_header_test2.c)
	TARGET_LINK_LIBRARIES(${header_base}_header_test2  csm-static ${csm_link_flags})
	
endforeach(header ${csm_headers} ${lib_headers})
endif(0)
