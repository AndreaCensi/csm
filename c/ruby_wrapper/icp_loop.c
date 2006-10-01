#include "laser_data.h"

#include "icp.h"
#include "journal.h"

void icp(struct icp_input*params, struct icp_output*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
		
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);
	
	ld_create_jump_tables(laser_ref);
	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);
	
	int iteration;
	
	gsl_vector * x_old = vector_from_array(3, params->odometry);
	gsl_vector * x_new = gsl_vector_alloc(3);
	
	for(iteration=0; iteration<params->maxIterations;iteration++) {
		fprintf(jf(), "iteration %i\n", iteration);
		journal_pose("x_old", x_old);
		
		find_correspondences(params, x_old);
		
		journal_correspondences(&(params->laser_sens));
		
		journal_pose("x_new", x_new);
		
	}
}

