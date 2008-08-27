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
	
	int all_is_okay = 1;
	
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
		double fail_perc = 0.05;
		if(num_corr < fail_perc * laser_sens->nrays) { /* TODO: arbitrary */
			sm_error("Failed: before trimming, only %d correspondences.\n",num_corr);
			all_is_okay = 0;
			egsl_pop(); /* loop context */
			break;
		}

		if(JJ) jj_add("corr0", corr_to_json(laser_sens->corr, laser_sens->nrays));

		/* Kill some correspondences (using dubious algorithm) */
		if(params->outliers_remove_doubles)
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
		if(num_corr_after < fail_perc * laser_sens->nrays){
			sm_error("Failed: after trimming, only %d correspondences.\n",num_corr_after);
			all_is_okay = 0;
			egsl_pop(); /* loop context */
			break;
		}

		/* Compute next estimate based on the correspondences */
		if(!compute_next_estimate(params, x_old, x_new)) {
			sm_error("Cannot compute next estimate.\n");
			all_is_okay = 0;
			egsl_pop(); /* loop context */
			break;			
		}

		pose_diff_d(x_new, x_old, delta);
		
		{
			sm_debug("killing %d rays valid,  %d corr found -> %d after double cut -> %d after adaptive cut \n", count_equal(laser_sens->valid, laser_sens->nrays, 1), num_corr, num_corr2, num_corr_after);
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

		
		/** PLICP terminates in a finite number of steps! */
		if(params->use_point_to_line_distance) {
			int loop_detected = 0; /* TODO: make function */
			int a; for(a=iteration-1;a>=0;a--) {
				if(hashes[a]==hashes[iteration]) {
					sm_debug("icpc: oscillation detected (cycle length = %d)\n", iteration-a);
					loop_detected = 1;
					break;
				}
			}
			if(loop_detected) break;
		}
	
		/* This termination criterium is useless when using
		   the point-to-line-distance; however, we put it here because
		   one can choose to use the point-to-point distance. */
		if(termination_criterion(params, delta)) {
			egsl_pop();
			break;
		}
		
		copy_d(x_new, 3, x_old);
		copy_d(delta, 3, delta_old);
		
		
		egsl_pop();
	}

	if(JJ) jj_loop_exit();
	
	*iterations = iteration+1;
	
	return all_is_okay;
}

int termination_criterion(struct sm_params*params, const double*delta){
	double a = norm_d(delta);
	double b = fabs(delta[2]);
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

int compute_next_estimate(struct sm_params*params, 
	const double x_old[3], double x_new[3]) 
{
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	
	struct gpc_corr c[laser_sens->nrays];

	int i; int k=0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i))
			continue;
		
		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;

		c[k].valid = 1;
		
/*		if(params->use_point_to_line_distance) {*/
		if(laser_sens->corr[i].type == corr_pl) {

			c[k].p[0] = laser_sens->points[i].p[0];
			c[k].p[1] = laser_sens->points[i].p[1];
			c[k].q[0] = laser_ref->points[j1].p[0];
			c[k].q[1] = laser_ref->points[j1].p[1];

			double diff[2];
			diff[0] = laser_ref->points[j1].p[0]-laser_ref->points[j2].p[0];
			diff[1] = laser_ref->points[j1].p[1]-laser_ref->points[j2].p[1];
			double one_on_norm = 1 / sqrt(diff[0]*diff[0]+diff[1]*diff[1]);
			double normal[2];
			normal[0] = +diff[1] * one_on_norm;
			normal[1] = -diff[0] * one_on_norm;

			double cos_alpha = normal[0];
			double sin_alpha = normal[1];
						
			c[k].C[0][0] = cos_alpha*cos_alpha;
			c[k].C[1][0] = 
			c[k].C[0][1] = cos_alpha*sin_alpha;
			c[k].C[1][1] = sin_alpha*sin_alpha;
			
			/* Note: it seems that because of numerical errors this matrix might be
			   not semidef positive. */
			double det = c[k].C[0][0] * c[k].C[1][1] - c[k].C[0][1] * c[k].C[1][0];
			double trace = c[k].C[0][0] + c[k].C[1][1];
			
			int semidef = (det >= 0) && (trace>0);
			if(!semidef) {
	/*			printf("%d: Adjusting correspondence weights\n",i);*/
				double eps = -det;
				c[k].C[0][0] += sqrt(eps);
				c[k].C[1][1] += sqrt(eps);
			}
			
		} else {
			c[k].p[0] = laser_sens->points[i].p[0];
			c[k].p[1] = laser_sens->points[i].p[1];
			
			projection_on_segment_d(
				laser_ref->points[j1].p,
				laser_ref->points[j2].p,
				laser_sens->points_w[i].p,
				c[k].q);
			
			double factor = 1;
			if(params->use_ml_weights) {
				double alpha = laser_ref->true_alpha[j1];
				if(!is_nan(alpha)) {
					double pose_theta = x_old[2];
					/** Incidence of the ray */
					double beta = alpha - (pose_theta+laser_sens->theta[i]);
					factor = 1 / square(cos(beta));
				}
			} 
			
			c[k].C[0][0] = factor;
		 	c[k].C[1][0] = 
		 	c[k].C[0][1] = 0;
		 	c[k].C[1][1] = factor;
		}
		
		k++;
	}
	
	/* TODO: use prior for odometry */
	double std = 0.11;
	const double inv_cov_x0[9] = 
		{1/(std*std), 0, 0,
		 0, 1/(std*std), 0,
		 0, 0, 0};
	
	
	int ok = gpc_solve(k, c, 0, inv_cov_x0, x_new);
	if(!ok) {
		sm_error("gpc_solve_valid failed");
		return 0;
	}

	double old_error = gpc_total_error(c, k, x_old);
	double new_error = gpc_total_error(c, k, x_new);

	sm_debug("Old error: %f  x_old= %s \n", old_error, friendly_pose(x_old));
	sm_debug("New error: %f  x_new= %s \n", old_error, friendly_pose(x_old));

	double epsilon = 0.000001;
	if(new_error > old_error + epsilon) {
		sm_error("Something's fishy here! Old error: %lf  new error: %lf  %lf %lf %lf  %lf %lf %lf\n",old_error,new_error,x_old[0],x_old[1],x_old[2],x_new[0],x_new[1],x_new[2]);
	}
	
	return 1;
}
	


