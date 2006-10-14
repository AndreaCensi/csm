%module sm

%header %{
	#include "rb_sm.h"
%}


struct sm_params {
	double maxAngularCorrectionDeg;
	double maxLinearCorrection;
	int maxIterations;
	double epsilon_xy;
	double epsilon_theta;
	double maxCorrespondenceDist;
	
	int restart;
	double restart_threshold_mean_error;
	double restart_dt;
	double restart_dtheta;
	
	double sigma;
	
	double clusteringThreshold;
	int orientationNeighbourhood;
	
	int doAlphaTest;
	double doAlphaTest_thresholdDeg;
	
	double outliers_maxPerc;
	
	int doVisibilityTest;
	int useCorrTricks;
	int doComputeCovariance;
	
};

struct sm_result {
	double x[3];
	int iterations;
	double error;
	int nvalid;
};


void rb_sm_init_journal(const char*journal_file);

void rb_sm_l_nrays(int laser, int nrays);
void rb_sm_l_min_theta(int laser, double);
void rb_sm_l_max_theta(int laser, double);
void rb_sm_l_ray(int laser, int ray, int valid, double theta, double reading);

void rb_sm_odometry(double x, double y, double theta);
void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta);

void rb_sm_icp();
void rb_sm_gpm();

void rb_sm_cleanup();


void rb_sm_get_x(double *OUTPUT,double*OUTPUT,double*OUTPUT);

%inline {
extern struct sm_params rb_sm_params;
extern struct sm_result rb_sm_result;
}