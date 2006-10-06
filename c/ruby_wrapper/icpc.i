%module icpc

%header %{
	#include "icp_ruby.h"
%}


void icpc_init_journal(const char*journal_file);

void icpc_l_nrays(int laser, int nrays);
void icpc_l_min_theta(int laser, double);
void icpc_l_max_theta(int laser, double);
void icpc_l_ray(int laser, int ray, double theta, double reading);

void icpc_odometry(double x, double y, double theta);
void icpc_odometry_cov(double cov_x, double cov_y, double cov_theta);

void icpc_go();
void icpc_cleanup();

void gpmc_go();

struct icp_input {
	double maxAngularCorrectionDeg;
	double maxLinearCorrection;
	int maxIterations;
	double epsilon_xy;
	double epsilon_theta;
	double maxCorrespondenceDist;
	
	int restart;
	double restart_dt;
	double restart_dtheta;
	
};

struct icp_output {
	double x[3];
	int iterations;
	double error;
};

void icpc_get_x(double *OUTPUT,double*OUTPUT,double*OUTPUT);

%inline {
extern struct icp_input icpc_params;
extern struct icp_output icpc_res;
}