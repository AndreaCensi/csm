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

	double maxAngularCorrectionDeg;
	double maxLinearCorrection;

	/** When to stop */
	int maxIterations;
	double epsilon_xy;
	double epsilon_theta;
	
	/** dubious parameters */
	double maxCorrespondenceDist;
	
	double sigma;
	
	int useCorrTricks;
	
	int restart;
	double restart_threshold_mean_error;
	double restart_dt;
	double restart_dtheta;
	
	double clusteringThreshold;
	int orientationNeighbourhood;
	
	int doAlphaTest;
	double doAlphaTest_thresholdDeg;
	
	// Percentage of correspondences to consider
	double outliers_maxPerc;

	double outliers_adaptive_order; // 0.7
	double outliers_adaptive_mult; // 2
	
	int doVisibilityTest;
	
	int doComputeCovariance;
	
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



