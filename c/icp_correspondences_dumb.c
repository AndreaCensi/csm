#include <gsl/gsl_vector.h>
#include "icp.h"


void find_correspondences(struct icp_input*params, gsl_vector* x_old) {
	struct laser_data* laser_ref  = &(params->laser_ref);
	struct laser_data* laser_sens = &(params->laser_sens);

	int i;
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	
	for(i=0;i<laser_sens->nrays;i++) {
		if(!valid_ray(laser_sens,i)) {
			set_null_correspondence(laser_sens,i);
			continue; 
		}

		transform(laser_sens->p[i], x_old, p_i_w);
		
		int j1 = -1;
		int best_dist = 0;
		
		int from; int to;
		possible_interval(p_i_w, laser_sens, params->maxAngularCorrectionDeg,
			params->maxLinearCorrection, &from, &to);
		
		int j;
		for(j=from;j<=to;j++) {
			if(!valid_ray(laser_ref,j)) continue;
			
			double dist = distance(p_i_w, laser_ref->p[j]);
			if(dist>params->maxCorrespondenceDist) continue;
			
			if((j1==-1) || (dist<best_dist)) {
				j1 =j; best_dist = dist;
			}
		}
		
		if(j1==-1) {// no match
			set_null_correspondence(laser_sens, i);
			continue;
		}
		
		int j2;
		int j2up   = next_valid_up   (laser_ref, j1);
		int j2down = next_valid_down (laser_ref, j1);
		if((j2up==-1)&&(j2down==-1)) {
			set_null_correspondence(laser_sens, i);
			continue;
		}
		if(j2up  ==-1) { j2 = j2down; } else
		if(j2down==-1) { j2 = j2up; } else {
			double dist_up   = distance(p_i_w, laser_ref->p[j2up  ]);
			double dist_down = distance(p_i_w, laser_ref->p[j2down]);
			j2 = dist_up < dist_down ? j2up : j2down;
		}
		
		set_correspondence(laser_sens, i, j1, j2);
	}
	
}
