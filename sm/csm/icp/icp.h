#ifndef _H_ICP_
#define _H_ICP_

#include <gpc/gpc.h>
#include "../csm_all.h"

void visibilityTest(LDP ld, const gsl_vector*x_old);

void compute_next_estimate(struct sm_params*params, LDP laser_ref, LDP laser_sens,  gsl_vector*x_new);
int termination_criterion(gsl_vector*delta, struct sm_params*params);
void find_correspondences(struct sm_params*params, gsl_vector* x_old);
void find_correspondences_tricks(struct sm_params*params, gsl_vector* x_old);
void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid);
int icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
 	double*total_error, int*nvalid, int*iterations);

void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old,
	double*total_error);
void kill_outliers_double(struct sm_params*params, const gsl_vector*x_old);
	
void compute_covariance_exact(
	LDP laser_ref, LDP laser_sens, const gsl_vector*x,
		val *cov0_x, val *dx_dy1, val *dx_dy2);

	/** Checks that find_correspondences_tricks and find_correspondences behave the same.
	 	Exit(-1) on error. */
	void debug_correspondences(struct sm_params * params, gsl_vector * x_old);

#endif
