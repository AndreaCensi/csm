#include <string.h>
#include "icp.h"

void debug_correspondences(struct sm_params * params, gsl_vector * x_old) {
	LDP laser_sens = params->laser_sens;
	/** Do the test */
	find_correspondences_tricks(params, x_old);
	struct correspondence c1[laser_sens->nrays];
	struct correspondence * c2 = laser_sens->corr;
	memcpy(c1, c2, sizeof(struct correspondence) * laser_sens->nrays);
	long hash1 = ld_corr_hash(laser_sens);
	find_correspondences(params, x_old);
	long hash2 = ld_corr_hash(laser_sens);
	if(hash1 != hash2) {
		sm_error("find_correspondences_tricks might be buggy\n");
		int i = 0; for(i=0;i<laser_sens->nrays;i++) {
			if( (c1[i].valid != c2[i].valid) ||
				(c1[i].j1 != c2[i].j1) || (c1[i].j2 != c2[i].j2) ) {
					sm_error("\tc1[%d].valid = %d j1 = %d  j2 = %d\n",
						i, c1[i].valid, c1[i].j1, c1[i].j2);
					sm_error("\tc2[%d].valid = %d j1 = %d  j2 = %d\n",
						i, c2[i].valid, c2[i].j1, c2[i].j2);
				}
		}
		if(1) exit(-1);
	}
}
