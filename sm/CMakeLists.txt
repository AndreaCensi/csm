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
		MESSAGE(WARNING " Cairo not found: carmen2pdf will not be built.")
		MESSAGE(WARNING " install Cairo from http://cairographics.org ")
	ENDIF(CAIRO_FOUND)
	


	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wall  -Wconversion  -Wunreachable-code ")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing")

	
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
	#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unreachable-code")
	

	IF(APPLE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-double")
		MESSAGE(STATUS "Using optimizations: ")
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450 -O3")
#		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -fomit-frame-pointer -mtune=7450 -mcpu=7450 -O3")
	ELSE(APPLE)
		# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
		# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -fomit-frame-pointer")

		#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=pentium3 -msse -O3 -pipe -fomit-frame-pointer")

	ENDIF(APPLE)


	########### 
	ADD_LIBRARY(gpc STATIC 
	 lib/gpc/gpc.c
	 lib/gpc/gpc_utils.c
	)
		 

	########### Options ########## 
	SUBDIRS(lib/options)
	SUBDIRS(lib/json-c)
	SUBDIRS(lib/egsl)
	SUBDIRS(csm)

	INCLUDE_DIRECTORIES(lib)
	INCLUDE_DIRECTORIES(.)

########### SM ########## 



	########### SM applications ########## 
	
	MACRO(new_executable exec)
		ADD_EXECUTABLE(${exec} apps/${exec}.c)
		TARGET_LINK_LIBRARIES(${exec} csm ${GSL_LIBRARIES} options)
		INSTALL(PROGRAMS ${exec} DESTINATION bin)
	ENDMACRO(new_executable exec)
	
	new_executable(sm0)
	new_executable(sm1)
	new_executable(sm2)
	new_executable(sm0)
	new_executable(json_extract)
	new_executable(carmen2json)
	new_executable(ld_noise)
	new_executable(ld_slip)
	new_executable(ld_fisher)

	new_executable(test_json_ld)
	new_executable(test_json)

	ADD_EXECUTABLE(test_math_utils csm/math_utils_test.c)
	TARGET_LINK_LIBRARIES(test_math_utils csm ${GSL_LIBRARIES})


########### Carmen2pdf ########## 
IF(CAIRO_FOUND)
	new_executable(carmen2pdf)
	
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CAIRO_CFLAGS}")
	TARGET_LINK_LIBRARIES(carmen2pdf -lcairo)
ENDIF(CAIRO_FOUND)
