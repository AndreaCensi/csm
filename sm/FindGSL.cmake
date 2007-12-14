## 
## Try to find gnu scientific library GSL  
## (see http://www.gnu.org/software/gsl/)
## Once run this will define: 
## 
## GSL_FOUND       = system has GSL lib
##
## GSL_LIBRARIES   = full path to the libraries
##    on Unix/Linux with additional linker flags from "gsl-config --libs"
## 
## CMAKE_GSL_CXX_FLAGS  = Unix compiler flags for GSL, essentially "`gsl-config --cxxflags`"
##
## GSL_INCLUDE_DIR      = where to find headers 
##
## GSL_LINK_DIRECTORIES = link directories, useful for rpath on Unix
## GSL_EXE_LINKER_FLAGS = rpath on Unix
##
## Felix Woelk 07/2004
## www.mip.informatik.uni-kiel.de
## --------------------------------

 IF(WIN32 AND NOT CYGWIN)
 MESSAGE(STATUS, "Finding GSL using the WIN32 code.")
  FIND_LIBRARY(GSL_gsl_LIBRARY
      NAMES gsl
      PATHS "$ENV{GSL_HOME}/lib"
      DOC "Where can the GSL (gsl.lib) library be found"
      )
  FIND_LIBRARY(GSL_cblas_LIBRARY
      NAMES cblas
      PATHS "$ENV{GSL_HOME}/lib"
      DOC "Where can the GSL (cblas.lib) library be found"
      )
  SET(GSL_LIBRARIES "${GSL_cblas_LIBRARY} ${GSL_gsl_LIBRARY}")

  FIND_PATH(GSL_INCLUDE_DIR gsl/gsl_linalg.h
      $ENV{GSL_HOME}/include
      )

  IF(GSL_INCLUDE_DIR AND GSL_LIBRARIES)
    SET(GSL_FOUND TRUE)
  ELSE(GSL_INCLUDE_DIR AND GSL_LIBRARIES)
    SET(GSL_FOUND FALSE) 
  ENDIF(GSL_INCLUDE_DIR AND GSL_LIBRARIES)

  MARK_AS_ADVANCED(
    GSL_gsl_LIBRARY
    GSL_cblas_LIBRARY
    GSL_INCLUDE_DIR
    GSL_LIBRARIES
    GSL_LINK_DIRECTORIES
  )  
ELSE(WIN32 AND NOT CYGWIN)
   MESSAGE("gsl home: $ENV{GSL_HOME}")
#  IF(UNIX) 
  SET(GSL_CONFIG_PREFER_PATH "$ENV{GSL_HOME}/bin" CACHE STRING "preferred path to GSL (gsl-config)")
  IF(NOT GSL_CONFIG) 
   FIND_PROGRAM(GSL_CONFIG gsl-config
      ${GSL_CONFIG_PREFER_PATH}
      /usr/bin/
      )
   ENDIF(NOT GSL_CONFIG)

   MESSAGE("DBG GSL_CONFIG ${GSL_CONFIG}")
    
    IF (GSL_CONFIG) 
      # set CXXFLAGS to be fed into CXX_FLAGS by the user:
     #  SET(GSL_CXX_FLAGS "`${GSL_CONFIG} --cflags`")
      EXEC_PROGRAM(${GSL_CONFIG} ARGS --cflags OUTPUT_VARIABLE GSL_CXX_FLAGS)
	
      # set INCLUDE_DIRS to prefix+include
      EXEC_PROGRAM(${GSL_CONFIG}
	ARGS --prefix
	OUTPUT_VARIABLE GSL_PREFIX)
      SET(GSL_INCLUDE_DIR ${GSL_PREFIX}/include CACHE STRING INTERNAL)
	  
      # set link libraries and link flags
      # SET(GSL_LIBRARIES "`${GSL_CONFIG} --libs`")
      EXEC_PROGRAM(${GSL_CONFIG} ARGS --libs OUTPUT_VARIABLE GSL_LIBRARIES)
      
      ## extract link dirs for rpath  
      EXEC_PROGRAM(${GSL_CONFIG}
	ARGS --libs
	OUTPUT_VARIABLE GSL_CONFIG_LIBS )

      ## split off the link dirs (for rpath)
      ## use regular expression to match wildcard equivalent "-L*<endchar>"
      ## with <endchar> is a space or a semicolon
      STRING(REGEX MATCHALL "[-][L]([^ ;])+" 
	GSL_LINK_DIRECTORIES_WITH_PREFIX 
	"${GSL_CONFIG_LIBS}" )
#      MESSAGE("DBG  GSL_LINK_DIRECTORIES_WITH_PREFIX=${GSL_LINK_DIRECTORIES_WITH_PREFIX}")
      
      ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
     
      IF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
	STRING(REGEX REPLACE "[-][L]" "" GSL_LINK_DIRECTORIES ${GSL_LINK_DIRECTORIES_WITH_PREFIX} )
      ENDIF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
      SET(GSL_EXE_LINKER_FLAGS "-Wl,-rpath,${GSL_LINK_DIRECTORIES}" CACHE STRING INTERNAL)
#      MESSAGE("DBG  GSL_LINK_DIRECTORIES=${GSL_LINK_DIRECTORIES}")
#      MESSAGE("DBG  GSL_EXE_LINKER_FLAGS=${GSL_EXE_LINKER_FLAGS}")

      ADD_DEFINITIONS("-DHAVE_GSL")
      SET(GSL_DEFINITIONS "-DHAVE_GSL")
      MARK_AS_ADVANCED(
	GSL_CXX_FLAGS
	GSL_INCLUDE_DIR
	GSL_LIBRARIES
	GSL_LINK_DIRECTORIES
	GSL_DEFINITIONS
	)
      MESSAGE(STATUS "Using GSL from ${GSL_PREFIX}")
      
    ELSE(GSL_CONFIG)
      MESSAGE(FATAL_ERROR, "FindGSL.cmake: gsl-config not found. Please set it manually. GSL_CONFIG=${GSL_CONFIG}")
    ENDIF(GSL_CONFIG)

#  ENDIF(UNIX)
ENDIF(WIN32 AND NOT CYGWIN)


IF(GSL_LIBRARIES)
  IF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)

    SET(GSL_FOUND 1)
    
  ENDIF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)
ENDIF(GSL_LIBRARIES)


