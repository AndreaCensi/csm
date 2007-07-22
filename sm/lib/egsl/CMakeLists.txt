
SET(egsl_sources 
	egsl.c 
	egsl_ops.c
	egsl_conversions.c
	egsl_misc.c
)

ADD_LIBRARY(egsl STATIC ${egsl_sources})

ADD_EXECUTABLE(egsl_test egsl_test.c)
TARGET_LINK_LIBRARIES(egsl_test egsl  ${GSL_LIBRARIES})

ADD_EXECUTABLE(egsl_test_allocation egsl_test_allocation.c)
TARGET_LINK_LIBRARIES(egsl_test_allocation egsl  ${GSL_LIBRARIES})


# installation

FILE(GLOB headers "*.h")
INSTALL(FILES ${headers} DESTINATION include/egsl)
INSTALL(TARGETS egsl ARCHIVE DESTINATION lib)
