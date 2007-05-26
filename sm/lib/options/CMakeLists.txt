ADD_LIBRARY(options STATIC options.c options_interface.c)

ADD_EXECUTABLE(test_options test_options.c)
TARGET_LINK_LIBRARIES(test_options options)
