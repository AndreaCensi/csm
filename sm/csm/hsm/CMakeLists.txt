# This builds the code as itself

cmake_minimum_required(VERSION 2.4)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -std=c99")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")


ADD_EXECUTABLE(hsm_test00 hsm_test00.c hsm.c)
