#include <math.h>
#include <string.h>

#include <gsl/gsl_matrix.h>

#include <gpc/gpc.h>
#include <egsl/egsl_macros.h>

#include "../csm_all.h"

#include "icp.h"

void sm_icp(struct sm_params*params, struct sm_result*res) {
	res->valid = 0;
	
	if(!ld_valid_fields(params->laser_ref) || 
	   !ld_valid_fields(params->laser_sens)) {
		return;
	}
		
	
	if(JJ) jj_context_enter("sm_icp");
	
	egsl_push();
	
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
			
	if(params->use_corr_tricks)
		ld_create_jump_tables(laser_ref);
		
	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);

	if(params->do_alpha_test) {
		ld_simple_clustering(laser_ref, params->clustering_threshold);
		ld_compute_orientation(laser_ref, params->orientation_neighbourhood, params->sigma);
		ld_simple_clustering(laser_sens, params->clustering_threshold);
		ld_compute_orientation(laser_sens, params->orientation_neighbourhood, params->sigma);
	}

	if(JJ) jj_add("laser_ref",  ld_to_json(laser_ref));
	if(JJ) jj_add("laser_sens", ld_to_json(laser_sens));
	
	/*journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);*/
	
	gsl_vector * x_new = gsl_vector_alloc(3);
	gsl_vector * x_old = vector_from_array(3, params->first_guess);
	
	if(params->do_visibility_test) {
		sm_debug("laser_ref:\n");
		visibilityTest(laser_ref, x_old);

		sm_debug("laser_sens:\n");
		gsl_vector * minus_x_old = gsl_vector_alloc(3);
		ominus(x_old,minus_x_old);
		visibilityTest(laser_sens, minus_x_old);
		gsl_vector_free(minus_x_old);
	}
	
	double error;
	int iterations;
	int nvalid;
	if(!icp_loop(params, x_old, x_new, &error, &nvalid, &iterations)) {
		sm_error("ICP not complete. \n");
		return;
	}

	double best_error = error;
	gsl_vector * best_x = gsl_vector_alloc(3);
	gsl_vector_memcpy(best_x, x_new);

	if(params->restart && 
		(error/nvalid)>(params->restart_threshold_mean_error) ) {
		sm_debug("Restarting: %f > %f \n",(error/nvalid),(params->restart_threshold_mean_error));
		double dt  = params->restart_dt;
		double dth = params->restart_dtheta;
		sm_debug("icp_loop: dt = %f dtheta= %f deg\n",dt,rad2deg(dth));
		
		double perturb[6][3] = {
			{dt,0,0}, {-dt,0,0},
			{0,dt,0}, {0,-dt,0},
			{0,0,dth}, {0,0,-dth}
		};

		int a; for(a=0;a<6;a++){
			sm_debug("-- Restarting with perturbation #%d\n", a);
			struct sm_params my_params = *params;
			gsl_vector * start = gsl_vector_alloc(3);
				gvs(start, 0, gvg(x_new,0)+perturb[a][0]);
				gvs(start, 1, gvg(x_new,1)+perturb[a][1]);
				gvs(start, 2, gvg(x_new,2)+perturb[a][2]);
			gsl_vector * x_a = gsl_vector_alloc(3);
			double my_error; int my_valid; int my_iterations;
			if(!icp_loop(&my_params, start, x_a, &my_error, &my_valid, &my_iterations)){
				sm_error("Error during restart #%d/%d. \n", a, 6);
				break;
			}
			iterations+=my_iterations;
		
			if(my_error < best_error) {
				sm_debug("--Perturbation #%d resulted in error %f < %f\n", a,my_error,best_error);
				gsl_vector_memcpy(best_x, x_a);
				best_error = my_error;
			}
			gsl_vector_free(x_a); gsl_vector_free(start);
		}
	}
	
	
	/* At last, we did it. */
	res->valid = 1;
	vector_to_array(best_x, res->x);
	sm_debug("icp: final x =  %s  \n", gsl_friendly_pose(best_x));
	
	
	if(params->do_compute_covariance)  {

		val cov0_x, dx_dy1, dx_dy2;
		compute_covariance_exact(
			laser_ref, laser_sens, best_x,
			&cov0_x, &dx_dy1, &dx_dy2);
		
		val cov_x = sc(square(params->sigma), cov0_x); 
/*		egsl_v2da(cov_x, res->cov_x); */
		
		res->cov_x_m = egsl_v2gslm(cov_x);
		res->dx_dy1_m = egsl_v2gslm(dx_dy1);
		res->dx_dy2_m = egsl_v2gslm(dx_dy2);
		
		if(0) {
			egsl_print("cov0_x", cov0_x);
			egsl_print_spectrum("cov0_x", cov0_x);
		
			val fim = ld_fisher0(laser_ref);
			val ifim = inv(fim);
			egsl_print("fim", fim);
			egsl_print_spectrum("ifim", ifim);
		}
	}
	
	
	res->error = best_error;
	res->iterations = iterations;
	res->nvalid = nvalid;

	gsl_vector_free(x_new);
	gsl_vector_free(x_old);
	gsl_vector_free(best_x);
	egsl_pop();

	if(JJ) jj_context_exit();
}


int icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
	double*total_error, int*valid, int*iterations) {
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	
	gsl_vector * x_old = vector_from_array(3, start->data);
	
	sm_debug("icp: starting at  x' =  %s  \n", gsl_friendly_pose(x_old));
	
	gsl_vector * delta = gsl_vector_alloc(3);
	gsl_vector * delta_old = gsl_vector_alloc(3);
	gsl_vector_set_all(delta_old, 0.0);

	unsigned int hashes[params->max_iterations];
	int iteration;

	sm_debug("icp_loop: starting at x_old= %s  \n",
		gsl_friendly_pose(x_old));
	
	if(JJ) jj_loop_enter("iterations");
	
	for(iteration=0; iteration<params->max_iterations;iteration++) {
		if(JJ) jj_loop_iteration();

		egsl_push();
		
/*		if(jf()) fprintf(jf(), "iteration %d\n", iteration);*/

		if(JJ) jj_add("x_old", vector_to_json(x_old));
		
		if(params->use_corr_tricks)
			find_correspondences_tricks(params, x_old);
		else
			find_correspondences(params, x_old);

		if(params->debug_verify_tricks)
			debug_correspondences(params, x_old);

		int num_corr = ld_num_valid_correspondences(laser_sens);
		if(num_corr <0.2 * laser_sens->nrays){
			egsl_pop();
			sm_error("Failed: before trimming, only %d correspondences.\n",num_corr);
			return 0;
		}

		if(JJ) jj_add("corr0", corr_to_json(laser_sens->corr, laser_sens->nrays));

		kill_outliers_double(params);
		int num_corr2 = ld_num_valid_correspondences(laser_sens);

		if(JJ) jj_add("corr1", corr_to_json(laser_sens->corr, laser_sens->nrays));
		
		double error=0;
		kill_outliers_trim(params, x_old, &error);
		int num_corr_after = ld_num_valid_correspondences(laser_sens);
		
		if(JJ) jj_add("corr2", corr_to_json(laser_sens->corr, laser_sens->nrays));

		if(JJ) {
			jj_add_int("num_corr0", num_corr);
			jj_add_int("num_corr1", num_corr2);
			jj_add_int("num_corr2", num_corr_after);
		}

		*total_error = error; 
		*valid = num_corr_after;
		
		if(num_corr_after <0.2 * laser_sens->nrays){
			sm_error("Failed: after trimming, only %d correspondences.\n",num_corr_after);
			egsl_pop();
			return 0;
		}

		
		compute_next_estimate(params, laser_ref, laser_sens,	 x_new);		
		pose_diff(x_new, x_old, delta);
		
		{
			/*journal_correspondences(laser_sens);*/
			sm_debug("killing %d -> %d -> %d \n", num_corr, num_corr2, num_corr_after);
			/*journal_pose("x_new", x_new);
			journal_pose("delta", delta);*/
			if(JJ) {
				jj_add("x_new", vector_to_json(x_new));
				jj_add("delta", vector_to_json(delta));
			}
		}
		/** Checks for oscillations */
		hashes[iteration] = ld_corr_hash(laser_sens);
		
		{
			sm_debug("icp_loop: it. %d  hash=%d nvalid=%d mean error = %f, x_new= %s\n", 
				iteration, hashes[iteration], *valid, *total_error/ *valid, 
				gsl_friendly_pose(x_new));
		}

		egsl_pop();
						
		int detected = 0;
		int a; for(a=0;a<iteration;a++) {
			if(hashes[a]==hashes[iteration]) {
				sm_debug("icpc: oscillation detected (cycle length = %d)\n", iteration-a);
				detected = 1;
			}
		}
		if(detected) break;

		if(termination_criterion(delta, params)) 
			break;
		
		gsl_vector_memcpy(x_old, x_new);
		gsl_vector_memcpy(delta_old, delta);
	}

	if(JJ) jj_loop_exit();
	
	/* TODO: covariance */
	*iterations = iteration+1;
	
	gsl_vector_free(x_old);
	gsl_vector_free(delta);
	gsl_vector_free(delta_old);

	return 1;
}

int termination_criterion(gsl_vector*delta, struct sm_params*params){
	double a = sqrt(gvg(delta,0)* gvg(delta,0)+ gvg(delta,1)* gvg(delta,1));
	double b = fabs(gvg(delta,2));
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

void compute_next_estimate(struct sm_params*params, LDP laser_ref, LDP laser_sens, gsl_vector*x_new) {
	struct gpc_corr c[laser_sens->nrays];

	int i; int k=0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i))
			continue;
		
		c[k].p[0] = gvg(laser_sens->p[i],0);
		c[k].p[1] = gvg(laser_sens->p[i],1);

		int j1 = laser_sens->corr[i].j1;
		c[k].q[0] = gvg(laser_ref->p[j1],0);
		c[k].q[1] = gvg(laser_ref->p[j1],1);
		
		int j2 = laser_sens->corr[i].j2;
		
		double diff[2];
		diff[0] = gvg(laser_ref->p[j1],0)-gvg(laser_ref->p[j2],0);
		diff[1] = gvg(laser_ref->p[j1],1)-gvg(laser_ref->p[j2],1);
		double one_on_norm = 1 / sqrt(diff[0]*diff[0]+diff[1]*diff[1]);
		double normal[2];
		normal[0] = +diff[1] * one_on_norm;
		normal[1] = -diff[0] * one_on_norm;
		
/*		double alpha = M_PI/2 + atan2( 
			gvg(laser_ref->p[j1],1)-gvg(laser_ref->p[j2],1),
			gvg(laser_ref->p[j1],0)-gvg(laser_ref->p[j2],0));*/

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
		 	/*c[k].C[0][0] += 0.02;
			c[k].C[1][1] += 0.02; */
		}
		
		k++;
	}
	
/*	const double x0[3] = {0, 0, 0}; */
	double std = 0.11;
	const double inv_cov_x0[9] = 
		{1/(std*std), 0, 0,
		 0, 1/(std*std), 0,
		 0, 0, 0};
	
	double x[3];
	gpc_solve_valid(k, c, 0, 0, inv_cov_x0, x);
	
	gvs(x_new,0,x[0]);
	gvs(x_new,1,x[1]);
	gvs(x_new,2,x[2]);
}
	


