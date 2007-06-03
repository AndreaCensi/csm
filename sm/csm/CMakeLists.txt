ADD_LIBRARY(csm STATIC 
	journal.c
	laser_data.c
	laser_data_fisher.c
	laser_data_json.c
	laser_data_carmen.c
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

	gpm/gpm.c
)

TARGET_LINK_LIBRARIES(csm json-c egsl gpc)
TARGET_LINK_LIBRARIES(csm ${GSL_LIBRARIES})

INSTALL(TARGETS csm ARCHIVE DESTINATION lib)
INSTALL(FILES csm.h  DESTINATION include/csm)
INSTALL(FILES laser_data.h  DESTINATION include/csm)
