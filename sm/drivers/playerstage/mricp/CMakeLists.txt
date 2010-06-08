cmake_minimum_required(VERSION 2.8)

SET (mricpSrcs  src/geometry2D.cpp
                src/icp.cpp
                src/lasermodel.cpp
                src/map.cpp
                src/mricp_driver.cpp
                src/nn.cpp
                src/Timer.cpp)

MESSAGE("CSMPP_LIBRARY_DIRS: ${CSM_LIBRARY_DIRS}")
MESSAGE("CSMPP_LIBRARIES: ${CSM_LIBRARIES}")
MESSAGE("CSMPP_INCLUDE_DIRS: ${CSM_INCLUDE_DIRS}")
MESSAGE("CSMPP_LDFLAGS: " ${CSM_LDFLAGS})
INCLUDE_DIRECTORIES(${CSM_INCLUDE_DIRS})
LINK_DIRECTORIES(${CSM_LIBRARY_DIRS})

find_package(PkgConfig REQUIRED)

pkg_check_modules(PLAYERSTAGE REQUIRED playercore playerc playerinterface playerdrivers playerc++ playercommon)
IF(PLAYERSTAGE_FOUND)
        #do something ?
ELSE(PLAYERSTAGE_FOUND)
        MESSAGE("Cannot find PlayerStage version 3.0 or above")
ENDIF(PLAYERSTAGE_FOUND)

pkg_check_modules(GTK REQUIRED gtk+-2.0)
IF(GTK_FOUND)
	#do something ?
ELSE(GTK_FOUND)
	MESSAGE("Cannot find GTK version 2.0")
ENDIF(GTK_FOUND)


pkg_check_modules(GDKPIXBUF REQUIRED gdk-pixbuf-2.0)
IF(GDKPIXBUF_FOUND)
	#do something ?
ELSE(GDKPIXBUF_FOUND)
	MESSAGE("Cannot find GDKPIXBUF version 2.0")
ENDIF(GDKPIXBUF_FOUND)

ADD_LIBRARY(mricp SHARED ${mricpSrcs})

Message(" player stage: ${PLAYERSTAGE_LIBRARIES}")

TARGET_LINK_LIBRARIES(mricp
  ${PLAYERSTAGE_LIBRARIES}
  ${CSM_LIBRARIES}
  ${GTK_LIBRARIES}
  ${GDKPIXBUF_LIBRARIES}
  ${CSM_LIBRARY_DIRS}
)

INCLUDE_DIRECTORIES(
    ${PLAYERSTAGE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
    ${GTK_INCLUDE_DIRS}
    ${GDKPIXBUF_INCLUDE_DIRS}
    ${CSM_INCLUDE_DIRS}
  ./include
  ../../../csmpp
  ../../../csm
)
