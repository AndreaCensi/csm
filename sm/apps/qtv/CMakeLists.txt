project(qtv)

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

#join(CSM_LDFLAGS "${CSM_LDFLAGS}")
join(CSM_CFLAGS "${CSM_CFLAGS}")

MESSAGE("CSM_CFLAGS: ${CSM_CFLAGS}")
MESSAGE("CSM_LDFLAGS: ${CSM_LDFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CSM_CFLAGS}")
SET(LDFLAGS ${CSM_LDFLAGS})


include(${QT_USE_FILE})

# the variable "qtproject_SRCS" contains all .cpp files of this project
set(qtv_SRCS
    qtv.cpp
    mouse.cpp
	qlaserdata.cpp
	qfilereader.cpp
	qparser.cpp
	qscene.cpp
	qmapview.cpp
)

qt4_automoc(${qtv_SRCS})

add_executable(qtv ${qtv_SRCS})

target_link_libraries(qtv ${QT_LIBRARIES})
target_link_libraries(qtv ${CSM_LDFLAGS})

