#ifndef H_ICP
#define H_ICP

#include <gsl/gsl_vector.h>

struct correspondence {
	int valid; 
	int j1; int j2;
};

struct laser_data {
	int nrays;
	double  min_theta;
	double  max_theta;
	double *readings;
	double *theta;

	
	int *cluster;
	
	double *alpha;
	double *cov_alpha;
	int *alpha_valid;
	
	/* Jump tables */
	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;

	/* Cartesian points */
	gsl_vector**p;
	
	struct correspondence* corr;
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
	
	int restart;
	double restart_dt;
	double restart_dtheta;
	
	double clusteringThreshold;
	int orientationNeighbourhood;
	
	int doAlphaTest;
	double doAlphaTest_thresholdDeg;
	
	// Percentage of correspondences to consider
	double outliers_maxPerc;
};

struct sm_result {
	double x[3];
	double x_cov[3][3];
	int iterations;
	double error;
	
	double ** dx_dy1;
	double ** dx_dy2;
};


void icp(struct sm_params*input, struct sm_result*output);
void gpm(struct sm_params*input, struct sm_result*output);

void sm_journal_open(const char* file);

#endif



