#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__STRICT_ANSI_ansi")

ADD_LIBRARY(json-c STATIC
	arraylist.c
	debug.c
	json_object.c
	json_tokener.c
	json_util.c
	linkhash.c
	printbuf.c
	JSON_checker.c
	json_more_utils.c
)

#ADD_EXECUTABLE(test1 test1.c)
#ADD_EXECUTABLE(test2 test2.c)

#TARGET_LINK_LIBRARIES(test1 json-c)
#TARGET_LINK_LIBRARIES(test2 json-c)
