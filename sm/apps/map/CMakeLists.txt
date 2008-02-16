project(map)

macro(join ret list)
	SET(res "")
	FOREACH(e ${list})
		SET(res "${res} ${e}")
	ENDFOREACH(e ${list})
	SET(${ret} ${res})
endmacro(join list)


set(QT_USE_QTOPENGL 1)
find_package(Qt4 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CSM REQUIRED csm)

join(CSM_CFLAGS "${CSM_CFLAGS}")
MESSAGE("CSM_CFLAGS: ${CSM_CFLAGS}")
#MESSAGE("CSM_LDFLAGS: ${CSM_LDFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CSM_CFLAGS}")
SET(LDFLAGS ${CSM_LDFLAGS})

# the variable "qtproject_SRCS" contains all .cpp files of this project
set(map1_SRCS
	map1.cpp
)

qt4_automoc(${map1_SRCS})

add_executable(map1 ${map1_SRCS})

target_link_libraries(map1 ${QT_LIBRARIES})
target_link_libraries(map1 ${CSM_LDFLAGS})

