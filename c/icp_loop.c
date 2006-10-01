#include "laser_data.h"

#include "icp.h"
#include "journal.h"

void icp(struct icp_input*params, struct icp_output*res) {
	journal_laser_data("laser_ref",  &(params->laser_ref));
	journal_laser_data("laser_sens", &(params->laser_sens));
	
	create_jump_tables(&(params->laser_ref));
	
	struct pose x_old;
	
	x_old = params->odometry;
	int iteration;
	for(iteration=0; iteration<params->maxIterations;iteration++) {
		
		
		
	}
}

