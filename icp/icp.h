#ifndef H_ICP
#define H_ICP

#include "laser_data.h"

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
	
	int restart;
	double restart_dt;
	double restart_dtheta;
};

struct icp_output {
	double x[3];
	double x_cov[3][3];
	int iterations;
	double error;
	
	double ** dx_dy1;
	double ** dx_dy2;
};



void icp(struct icp_input*input, struct icp_output*output);
void icp_journal_open(const char* file);

void gpm(struct icp_input*input, struct icp_output*output);

#endif







