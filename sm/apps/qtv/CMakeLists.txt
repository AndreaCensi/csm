project(qtv)

find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

# the variable "qtproject_SRCS" contains all .cpp files of this project
set(qtv_SRCS
    main.cpp
    mouse.cpp
)

qt4_automoc(${qtv_SRCS})

add_executable(qtv ${qtv_SRCS})

target_link_libraries(qtv ${QT_LIBRARIES})

