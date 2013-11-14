cmake_minimum_required(VERSION 2.6)

PROJECT(csm_eigen)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") 

if(WIN32)
  add_definitions(-DWINDOWS)
  if(MSVC)
    # SSE2 optimizations
    ADD_DEFINITIONS("/arch:SSE2")
  endif()
endif()

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE RelWithDebInfo)
#SET(CMAKE_BUILD_TYPE Release)

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(Eigen3 REQUIRED)
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include ${CMAKE_SOURCE_DIR})

SET(CSM_SRC 
  csm/laser_data.cpp
  csm/math_utils.cpp
  csm/math_utils_gsl.cpp
  csm/utils.cpp
  csm/logging.cpp
  csm/sm_options.cpp
  csm/orientation.cpp
  csm/clustering.cpp)

SET(EGSL_SRC
	egsl/egsl.cpp
	egsl/egsl_conversions.cpp
	egsl/egsl_misc.cpp
	egsl/egsl_ops.cpp)

SET(GPC_SRC
	gpc/gpc.cpp
	gpc/gpc_utils.cpp)

SET(ICP_SRC
  icp/icp.cpp
  icp/icp_corr_dumb.cpp
  icp/icp_corr_tricks.cpp
  icp/icp_covariance.cpp
  icp/icp_debug.cpp
  icp/icp_loop.cpp
  icp/icp_outliers.cpp)

SET(OPTIONS_SRC  
   options/options.cpp
   options/options_interface.cpp)

ADD_LIBRARY(csm_eigen
	${CSM_SRC}
	${EGSL_SRC} 
	${GPC_SRC}
	${ICP_SRC}
	${OPTIONS_SRC})
