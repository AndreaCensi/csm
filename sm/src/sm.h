#ifndef H_SCAN_MATCHING_LIB
#define H_SCAN_MATCHING_LIB

#include <gsl/gsl_vector.h>

struct correspondence {
	int valid; 
	int j1; int j2;
};

struct laser_data {
	int nrays;
	double  min_theta;
	double  max_theta;
	
	double *theta;
	
	int*valid;
	double *readings;
	
	
	int *cluster;
	
	double *alpha;
	double *cov_alpha;
	int *alpha_valid;
	
	/* Jump tables */
	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;

	/* Cartesian points */
	gsl_vector**p;
	
	struct correspondence* corr;

	
	double odometry[3];	
	double estimate[3];	
};

void ld_alloc(struct laser_data*, int nrays);
void ld_free(struct laser_data*);

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



