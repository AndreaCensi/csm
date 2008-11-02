#include <options/options.h>
#include <assert.h>

#include "../csm_all.h"

void hsm_add_options(struct option* ops, struct hsm_params*p) {
	options_double(ops, "hsm_linear_cell_size", &p->linear_cell_size, 0.03, "HSM: Size of a rho cell");
	options_double(ops, "hsm_angular_cell_size_deg", &p->angular_cell_size_deg, 1.0, "HSM: Size of angualar cell (deg)");
	options_int(ops, "hsm_num_angular_hypotheses", &p->num_angular_hypotheses, 8, "HSM: Number of angular hypotheses.");
	options_double(ops, "hsm_xc_directions_min_distance_deg", &p->xc_directions_min_distance_deg, 10.0, "HSM: Min distance between directions for cross corr (deg)");
	options_int(ops, "hsm_xc_ndirections", &p->xc_ndirections, 3, "HSM: Number of directions for cross corr (deg)");
	options_double(ops, "hsm_angular_hyp_min_distance_deg", &p->angular_hyp_min_distance_deg, 10.0, "HSM: Min distance between different angular hypotheses  (deg)");
	
	options_int(ops, "hsm_linear_xc_max_npeaks", &p->linear_xc_max_npeaks, 5, "HSM: Number of peaks per direction for linear translation");
	options_double(ops, "hsm_linear_xc_peaks_min_distance", &p->linear_xc_peaks_min_distance, 5.0, "HSM: Min distance between different peaks in linear correlation");
}

int hsm_compute_ht_for_scan(LDP ld, struct hsm_params*p, const double base[3], hsm_buffer *b);

int hsm_compute_ht_for_scan(LDP ld, struct hsm_params*p, const double base[3], hsm_buffer *b) {
	*b = 0;
	
	/** Find maximum reading for the points */
	double max_reading = max_in_array(ld->readings, ld->nrays);
	
	if(!(max_reading>0)) {
		sm_error("No valid points.\n");
		return 0;
	}
	
	p->max_norm = norm_d(base) + max_reading;
	
	*b = hsm_buffer_alloc(p);
	hsm_compute_ht_base(*b, base);
	
	ld_compute_cartesian(ld);
	int np = 0;
	for(int i=0; i<ld->nrays; i++) {
		if(!ld_valid_ray(ld, i)) continue;
		
		hsm_compute_ht_point(*b, ld->points[i].p[0], ld->points[i].p[1], 1.0);
		
		np++;
	}
	
	sm_debug("Computed HT with %d points.\n", np);
	if(np<5) {
		hsm_buffer_free(*b);
		*b = 0;
		return 0;
	} else {
		return 1;
	}
}
	
void sm_hsm(struct sm_params* params, struct sm_result* res) {
	res->valid = 0;
	
	params->first_guess[0]=0.2;
	params->first_guess[1]=0;
	params->first_guess[2]=0;
	

	/* use true information if present */
	int has_true1 = !any_nan(params->laser_ref->true_pose, 3);
	int has_true2 = !any_nan(params->laser_sens->true_pose, 3);
	if(has_true1 && has_true2) {
		params->hsm.debug_true_x_valid = 1;
		
		double true_x[3];
		pose_diff_d(params->laser_sens->true_pose, params->laser_ref->true_pose, true_x);

		/* This is the difference between results and true_x */
		pose_diff_d(true_x, params->first_guess,  params->hsm.debug_true_x);
		
	} else {
		params->hsm.debug_true_x_valid = 0;
	}
	
	double zero[3] = {0,0,0};
	hsm_buffer b1, b2;
	int ok1 = hsm_compute_ht_for_scan(params->laser_ref, &(params->hsm), zero, &b1);
	int ok2 = hsm_compute_ht_for_scan(params->laser_sens,&(params->hsm),  params->first_guess, &b2);
	
	if(!ok1 || !ok2) {
		sm_error("Could not compute buffers (too few points?).\n");
		if(b1) hsm_buffer_free(b1);
		if(b2) hsm_buffer_free(b2);
		return;
	}

	hsm_compute_spectrum(b1);
	hsm_compute_spectrum(b2);

	params->hsm.max_translation = max(b1->rho_max, b2->rho_max);
	
	hsm_match(&(params->hsm),b1,b2);


	if(b1->num_valid_results)	{
		res->valid = 1;
		double pl[3];
		double d2[3];
		
		pose_diff_d(params->first_guess, b1->results[0], res->x);
		pose_diff_d(b1->results[0], params->first_guess,  d2);
		oplus_d(params->first_guess, b1->results[0], pl);
		
		sm_info("hsm: odo   = %s\n", friendly_pose(params->first_guess));
		sm_info("hsm: res   = %s\n", friendly_pose(b1->results[0]));
		sm_info("hsm: plus  = %s\n", friendly_pose(pl));
		sm_info("hsm: d2  = %s\n", friendly_pose(d2));
		sm_info("hsm: xmin  = %s\n", friendly_pose(res->x));
		res->error = 0;
		res->iterations = 0;
		res->nvalid = 0;
	} else {
		sm_error("HSM did not produce any result.\n");
		res->valid = 0;
	}
	
	
	hsm_buffer_free(b1);
	hsm_buffer_free(b2);
}

