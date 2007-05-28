#ifndef H_SCAN_MATCHING_LIB
#define H_SCAN_MATCHING_LIB
#include "laser_data.h"
#include <gsl/gsl_vector.h>

struct sm_params {
	struct laser_data laser_ref;
	struct laser_data laser_sens;

	/** Where to start */
 	double odometry[3]; 
 	double odometry_cov[3][3]; 

	double max_angular_correction_deg;
	double max_linear_correction;

	/** When to stop */
	int max_iterations;
	double epsilon_xy;
	double epsilon_theta;
	
	/** dubious parameters */
	double max_correspondence_dist;
	
	double sigma;
	
	int use_corr_tricks;
	
	int restart;
	double restart_threshold_mean_error;
	double restart_dt;
	double restart_dtheta;
	
	double clustering_threshold;
	int orientation_neighbourhood;
	
	int do_alpha_test;
	double do_alpha_test_thresholdDeg;
	
	/** Percentage of correspondences to consider */
	double outliers_maxPerc;

	double outliers_adaptive_order; /* 0.7 */
	double outliers_adaptive_mult; /* 2 */
	
	int do_visibility_test;
	
	int do_compute_covariance;
	
};

struct sm_result {
	double x[3];
	double x_cov[3][3];
	int iterations;
	int nvalid;
	double error;
	
	double ** dx_dy1;
	double ** dx_dy2;
};


void sm_icp(struct sm_params*input, struct sm_result*output);
void sm_gpm(struct sm_params*input, struct sm_result*output);

void sm_journal_open(const char* file);

#endif



