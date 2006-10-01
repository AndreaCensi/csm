#ifndef H_ICP
#define H_ICP

#include "laser_data.h"
#include "math_utils.h"

struct icp_input {
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
};

struct icp_output {
	double x[3];
	double x_cov[3][3];
	double ** dx_dy1;
	double ** dx_dy2;
};

void find_correspondences(struct icp_input*params, gsl_vector* x_old);

void icp(struct icp_input*input, struct icp_output*output);


#endif