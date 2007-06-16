#include <math.h>
#include <string.h>

#include <gsl/gsl_matrix.h>

#include <gpc/gpc.h>
#include <egsl/egsl_macros.h>

#include "../csm_all.h"

#include "icp.h"

int icp_loop(struct sm_params*params, const double*q0, double*x_new, 
	double*total_error, int*valid, int*iterations) {
	LDP laser_sens = params->laser_sens;
	double x_old[3], delta[3], delta_old[3] = {0,0,0};
	copy_d(q0, 3, x_old);
	unsigned int hashes[params->max_iterations];
	int iteration;
	
	sm_debug("icp: starting at  q0 =  %s  \n", friendly_pose(x_old));
	
	if(JJ) jj_loop_enter("iterations");
	
	for(iteration=0; iteration<params->max_iterations;iteration++) {
		if(JJ) jj_loop_iteration();
		if(JJ) jj_add_double_array("x_old", x_old, 3);

		egsl_push();

		/** Compute laser_sens's points in laser_ref's coordinates
		    by roto-translating by x_old */
		ld_compute_world_coords(laser_sens, x_old);

		/** Find correspondences (the naif or smart way) */
		if(params->use_corr_tricks)
			find_correspondences_tricks(params);
		else
			find_correspondences(params);

		/** If debug_verify_tricks, make sure that find_correspondences_tricks()
		    and find_correspondences() return the same answer */
			if(params->debug_verify_tricks)
				debug_correspondences(params);

		/* If not many correspondences, bail out */
		int num_corr = ld_num_valid_correspondences(laser_sens);
		if(num_corr < 0.2 * laser_sens->nrays) { /* TODO: arbitrary */
			egsl_pop();
			sm_error("Failed: before trimming, only %d correspondences.\n",num_corr);
			return 0;
		}

		if(JJ) jj_add("corr0", corr_to_json(laser_sens->corr, laser_sens->nrays));

		/* Kill some correspondences (using dubious algorithm) */
		kill_outliers_double(params);
		int num_corr2 = ld_num_valid_correspondences(laser_sens);

		if(JJ) jj_add("corr1", corr_to_json(laser_sens->corr, laser_sens->nrays));
		
		double error=0;
		/* Trim correspondences */
		kill_outliers_trim(params, &error);
		int num_corr_after = ld_num_valid_correspondences(laser_sens);
		
		if(JJ) {
			jj_add("corr2", corr_to_json(laser_sens->corr, laser_sens->nrays));
			jj_add_int("num_corr0", num_corr);
			jj_add_int("num_corr1", num_corr2);
			jj_add_int("num_corr2", num_corr_after);
		}

		*total_error = error; 
		*valid = num_corr_after;

		sm_debug("Total error: %f  valid %d   mean = %f\n", *total_error, *valid, *total_error/ *valid);
		
		/* If not many correspondences, bail out */
		if(num_corr_after <0.2 * laser_sens->nrays){
			sm_error("Failed: after trimming, only %d correspondences.\n",num_corr_after);
			egsl_pop();
			return 0;
		}

		/* Compute next estimate based on the correspondences */
		compute_next_estimate(params, x_new);
		pose_diff_d(x_new, x_old, delta);
		
		{
			sm_debug("killing %d -> %d -> %d \n", num_corr, num_corr2, num_corr_after);
			if(JJ) {
				jj_add_double_array("x_new", x_new, 3);
				jj_add_double_array("delta", delta, 3);
			}
		}
		/** Checks for oscillations */
		hashes[iteration] = ld_corr_hash(laser_sens);
		
		{
			sm_debug("icp_loop: it. %d  hash=%d nvalid=%d mean error = %f, x_new= %s\n", 
				iteration, hashes[iteration], *valid, *total_error/ *valid, 
				friendly_pose(x_new));
		}

		egsl_pop();
		
		int loop_detected = 0; /* TODO: make function */
		int a; for(a=iteration-1;a>=0;a--) {
			if(hashes[a]==hashes[iteration]) {
				sm_debug("icpc: oscillation detected (cycle length = %d)\n", iteration-a);
				loop_detected = 1;
				break;
			}
		}
		if(loop_detected) break;

		/* This termination criterium is useless when using
		   the point-to-line-distance; however, we put it here because
		   one can choose to use the point-to-point distance. */
		if(termination_criterion(params, delta)) 
			break;
		
		copy_d(x_new, 3, x_old);
		copy_d(delta, 3, delta_old);
	}

	if(JJ) jj_loop_exit();
	
	*iterations = iteration+1;
	
	return 1;
}

int termination_criterion(struct sm_params*params, const double*delta){
	double a = norm_d(delta);
	double b = fabs(delta[2]);
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

void compute_next_estimate(struct sm_params*params, double*x_new) {
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	
	struct gpc_corr c[laser_sens->nrays];

	int i; int k=0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i))
			continue;
		
		/* Note that these are NOT the current points_w */
		c[k].p[0] = laser_sens->points[i].p[0];
		c[k].p[1] = laser_sens->points[i].p[1];

		int j1 = laser_sens->corr[i].j1;
		c[k].q[0] = laser_ref->points[j1].p[0];
		c[k].q[1] = laser_ref->points[j1].p[1];
		
		int j2 = laser_sens->corr[i].j2;
		
		double diff[2];
		diff[0] = laser_ref->points[j1].p[0]-laser_ref->points[j2].p[0];
		diff[1] = laser_ref->points[j1].p[1]-laser_ref->points[j2].p[1];
		double one_on_norm = 1 / sqrt(diff[0]*diff[0]+diff[1]*diff[1]);
		double normal[2];
		normal[0] = +diff[1] * one_on_norm;
		normal[1] = -diff[0] * one_on_norm;

		if(params->use_point_to_line_distance) {
			double cos_alpha = normal[0];
			double sin_alpha = normal[1];
			
			c[k].C[0][0] = cos_alpha*cos_alpha;
			c[k].C[1][0] = 
			c[k].C[0][1] = cos_alpha*sin_alpha;
			c[k].C[1][1] = sin_alpha*sin_alpha;
		} else {
			c[k].C[0][0] = 1;
		 	c[k].C[1][0] = 
		 	c[k].C[0][1] = 0;
		 	c[k].C[1][1] = 1;
		}
		
		k++;
	}
	
	/* TODO: use prior for odometry */
	double std = 0.11;
	const double inv_cov_x0[9] = 
		{1/(std*std), 0, 0,
		 0, 1/(std*std), 0,
		 0, 0, 0};
	
	gpc_solve_valid(k, c, 0, 0, inv_cov_x0, x_new);
	
}
	


