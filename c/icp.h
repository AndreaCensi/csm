#ifndef H_ICP
#define H_ICP

#include "laser_data.h"
#include "math_utils.h"



struct pose {
	
};

struct pose_cov {
	// XXX
};

struct icp_input {
	struct laser_data laser_ref;
	struct laser_data laser_sens;

	// Where to start
	struct pose      odometry;
	struct pose_cov  odometry_cov;

	// parameters
	double maxAngularCorrectionDeg;
	double maxLinearCorrection;

	// When to stop
	int maxIterations;
	double epsilon_xy;
	double epsilon_theta;
	
	// dubious parameters
	double maxCorrespondenceDist;
};

struct icp_output {
	struct pose      x;
	struct pose_cov  x_cov;
	double ** dx_dy1;
	double ** dx_dy2;
};

void icp(struct icp_input*input, struct icp_output*output);


#endif