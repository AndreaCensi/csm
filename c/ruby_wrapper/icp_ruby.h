#ifndef H_ICP_RUBY
#define H_ICP_RUBY


/** Interface for interfacing with Ruby: there are no pointers around. */

void icpc_init_journal(const char*journal_file);

void icpc_l_nrays(int laser, int nrays);
void icpc_l_min_theta(int laser, double min_theta);
void icpc_l_max_theta(int laser, double max_theta);
void icpc_l_ray(int laser, int ray, double theta, double reading);

void icpc_odometry(double x, double y, double theta);
void icpc_odometry_cov(double cov_x, double cov_y, double cov_theta);

void icpc_go();

void icpc_cleanup();

void icpc_get_x(double *x,double*y,double*theta);

void gpmc_go();

#include "icp.h"

extern struct icp_input icpc_params;
extern struct icp_output icpc_res;

#endif
