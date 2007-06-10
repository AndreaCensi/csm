#include <gsl/gsl_vector.h>

#include "../csm_all.h"



int compatible(struct sm_params*params, int i, int j) {
	if(!params->do_alpha_test) return 1;

	double theta0 = 0; /* FIXME */
	if((params->laser_sens->alpha_valid[i]==0) ||
	 (params->laser_ref->alpha_valid[j]==0))
	 return 1;

	double alpha_i = params->laser_sens->alpha[i];
	double alpha_j = params->laser_ref->alpha[j];
	double tolerance = deg2rad(params->do_alpha_test_thresholdDeg);

	/** FIXME remove alpha test */
	double theta = angleDiff(alpha_j, alpha_i);
	if(fabs(angleDiff(theta,theta0))>
		tolerance+deg2rad(params->max_angular_correction_deg)) {
	 return 0;
	} else {
	 return 1;
	}
}

void find_correspondences(struct sm_params*params, gsl_vector* x_old) {
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;

/*	if(jf()) fprintf(jf(),"param max_correspondence_dist %f\n",params->max_correspondence_dist);
	if(jf()) fprintf(jf(),"param max_linear_correction %f\n",params->max_linear_correction);
	if(jf()) fprintf(jf(),"param max_angular_correction_deg %f\n",params->max_angular_correction_deg);*/
	
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_ray(laser_sens,i)) {
			ld_set_null_correspondence(laser_sens,i);
			continue; 
		}

		transform(laser_sens->p[i], x_old, p_i_w);
		
		int j1 = -1;
		double best_dist = 0;
		
		int from; int to; int start_cell;
		possible_interval(p_i_w, laser_ref, params->max_angular_correction_deg,
			params->max_linear_correction, &from, &to, &start_cell);

		int j;
		for(j=from;j<=to;j++) {
			if(!ld_valid_ray(laser_ref,j)) continue;
			
			double dist = distance_squared(p_i_w, laser_ref->p[j]);
			if(dist>square(params->max_correspondence_dist)) continue;
			
			
			if((j1==-1) || (dist<best_dist)) {
				if(compatible(params, i, j)) {
					j1 =j; 
					best_dist = dist;
				}
			} 
		}
		
		if(j1==-1) {/* no match */
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		/* Do not match with extrema*/
		if(j1==0 || (j1 == (laser_ref->nrays-1))) {/* no match */
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		
		int j2;
		int j2up   = ld_next_valid_up   (laser_ref, j1);
		int j2down = ld_next_valid_down (laser_ref, j1);
		if((j2up==-1)&&(j2down==-1)) {
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		if(j2up  ==-1) { j2 = j2down; } else
		if(j2down==-1) { j2 = j2up; } else {
			double dist_up   = distance_squared(p_i_w, laser_ref->p[j2up  ]);
			double dist_down = distance_squared(p_i_w, laser_ref->p[j2down]);
			j2 = dist_up < dist_down ? j2up : j2down;
		}
		
		ld_set_correspondence(laser_sens, i, j1, j2);
	}
	gsl_vector_free(p_i_w);
	
}

