SET(csm_sources
	laser_data.c
	laser_data_fisher.c
	laser_data_json.c
	laser_data_carmen.c
	laser_data_load.c
	laser_data_drawing.c
	
	json_journal.c
	math_utils.c
	utils.c
	orientation.c
	clustering.c
	logging.c
	sm_options.c
	
	icp/icp.c
	icp/icp_loop.c
	icp/icp_corr_dumb.c
	icp/icp_corr_tricks.c
	icp/icp_outliers.c
	icp/icp_covariance.c
	icp/icp_debug.c

	gpm/gpm.c
)

IF(CAIRO_FOUND)
	SET(csm_sources ${csm_sources} laser_data_cairo.c)
ENDIF(CAIRO_FOUND)

ADD_LIBRARY(csm SHARED  ${csm_sources})

# SET_TARGET_PROPERTIES(csm PROPERTIES STATIC_LIBRARY_FLAGS  " -static ")
TARGET_LINK_LIBRARIES(csm json-c egsl gpc options)
TARGET_LINK_LIBRARIES(csm ${GSL_LIBRARIES})

IF(CAIRO_FOUND)
	TARGET_LINK_LIBRARIES(csm cairo)
ENDIF(CAIRO_FOUND)

INSTALL(TARGETS csm 
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)

FILE(GLOB csm_headers "*.h")
foreach(header ${csm_headers})
	INSTALL(FILES ${header} DESTINATION include/csm)
endforeach(header ${csm_headers})
