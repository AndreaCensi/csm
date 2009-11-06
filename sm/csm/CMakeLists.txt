SET(csm_sources
	laser_data.c
	laser_data_fisher.c
	laser_data_json.c
	laser_data_carmen.c
	laser_data_load.c
	laser_data_drawing.c
	laser_data_bbox.c
	
	json_journal.c
	math_utils.c
	math_utils_gsl.c
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
	
	mbicp/mbicp_interface.c      
	mbicp/MbICP.c      
	mbicp/calcul.c     
	mbicp/percolate.c  
	mbicp/sp_matrix.c
	hsm/hsm.c
	hsm/hsm_interface.c
	
	gpm/gpm.c
)

IF(CAIRO_FOUND)
	SET(csm_sources ${csm_sources} laser_data_cairo.c)
ENDIF(CAIRO_FOUND)


foreach(dir . hsm mbicp icp gpm structprior)
FILE(GLOB csm_headers "${dir}/*.h")
foreach(header ${csm_headers} )
 	INSTALL(FILES ${header} DESTINATION include/csm/${dir})
endforeach(header ${csm_headers})
endforeach(dir . csm)



