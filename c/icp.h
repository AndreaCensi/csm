#ifndef H_ICP
#define H_ICP

struct laser_data {
	int nrays;
	double  min_theta;
	double  max_theta;
	double *readings;
	double *theta;
};

struct pose {
	double x; double y; double theta;
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

#endif