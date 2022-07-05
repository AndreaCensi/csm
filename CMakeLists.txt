## This CMakeLists.txt file exists only for building as ROS 3rd-party package. Building for other purpose is not tested. See https://github.com/AndreaCensi/csm/pull/10
cmake_minimum_required(VERSION 2.8)
project(csm)

if(WIN32)
  add_definitions(-DWINDOWS)
  if(MSVC)
    # SSE2 optimizations
    add_definitions("/arch:SSE2")
  endif()
endif()

find_package(Eigen3 REQUIRED)

include_directories(include src ${EIGEN3_INCLUDE_DIR})

set(CSM_SRC 
  src/csm/laser_data.cpp
  src/csm/math_utils.cpp
  src/csm/math_utils_gsl.cpp
  src/csm/utils.cpp
  src/csm/logging.cpp
  src/csm/sm_options.cpp
  src/csm/orientation.cpp
  src/csm/clustering.cpp)

set(EGSL_SRC
  src/egsl/egsl.cpp
  src/egsl/egsl_conversions.cpp
  src/egsl/egsl_misc.cpp
  src/egsl/egsl_ops.cpp)

set(GPC_SRC
  src/gpc/gpc.cpp
  src/gpc/gpc_utils.cpp)

set(ICP_SRC
  src/icp/icp.cpp
  src/icp/icp_corr_dumb.cpp
  src/icp/icp_corr_tricks.cpp
  src/icp/icp_covariance.cpp
  src/icp/icp_debug.cpp
  src/icp/icp_loop.cpp
  src/icp/icp_outliers.cpp)

set(OPTIONS_SRC  
   src/options/options.cpp
   src/options/options_interface.cpp)

add_library(${PROJECT_NAME}
  ${CSM_SRC}
  ${EGSL_SRC} 
  ${GPC_SRC}
  ${ICP_SRC}
  ${OPTIONS_SRC})

add_library(${PROJECT_NAME}-static STATIC 
  ${CSM_SRC}
  ${EGSL_SRC} 
  ${GPC_SRC}
  ${ICP_SRC}
  ${OPTIONS_SRC})

install(TARGETS ${PROJECT_NAME}
                ${PROJECT_NAME}-static 
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES package.xml DESTINATION share/csm)

# pkg-config
set(PKG_CONFIG_C_FLAGS "-I${EIGEN3_INCLUDE_DIR}")
configure_file(pkg-config/csm.pc.in ${CMAKE_CURRENT_BINARY_DIR}/csm.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/csm.pc DESTINATION lib/pkgconfig)