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

	/** Maximum angular displacement between scans (deg)*/
	double max_angular_correction_deg;
	/** Maximum translation between scans (m) */
	double max_linear_correction;

	/** When to stop */
	int max_iterations;
	/** A threshold for stopping. */
	double epsilon_xy;
	/** A threshold for stopping. */
	double epsilon_theta;
	
	/** dubious parameter */
	double max_correspondence_dist;
	
	/** Noise in the scan */
	double sigma;
	
	/** Use smart tricks for finding correspondences. */
	int use_corr_tricks;
	
	/** Restart if error under threshold */
	int restart;
		/** Threshold for restarting */
		double restart_threshold_mean_error;
		/** Displacement for restarting */
		double restart_dt;
		/** Displacement for restarting */
		double restart_dtheta;
	
	
	/** For now, a very simple max-distance clustering algorithm is used */
	double clustering_threshold;
	/** Number of neighbour rays used to estimate the orientation.*/
	int orientation_neighbourhood;

	/** Discard correspondences based on the angles */
	int do_alpha_test;
	double do_alpha_test_thresholdDeg;

	/** Percentage of correspondences to consider */
	double outliers_maxPerc;

	double outliers_adaptive_order; /* 0.7 */
	double outliers_adaptive_mult; /* 2 */

	int do_visibility_test;

	int use_point_to_line_distance;
	int do_compute_covariance;
	
};

struct sm_result {
	double x[3];
/*	double cov_x[9];*/
	
	int iterations;
	int nvalid;
	double error;
	
	#ifndef RUBY
/*	double ** dx_dy1;
	double ** dx_dy2;*/
		gsl_matrix *cov_x_m;	
		gsl_matrix *dx_dy1_m;
		gsl_matrix *dx_dy2_m;
	#endif
};


void sm_icp(struct sm_params*input, struct sm_result*output);
void sm_gpm(struct sm_params*input, struct sm_result*output);

void sm_journal_open(const char* file);

#endif



