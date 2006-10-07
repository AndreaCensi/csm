#ifndef H_ICP_RUBY
#define H_ICP_RUBY


/** Interface for interfacing with Ruby: there are no pointers around. */

void rb_sm_init_journal(const char*journal_file);

void rb_sm_l_nrays(int laser, int nrays);
void rb_sm_l_min_theta(int laser, double min_theta);
void rb_sm_l_max_theta(int laser, double max_theta);
void rb_sm_l_ray(int laser, int ray, double theta, double reading);

void rb_sm_odometry(double x, double y, double theta);
void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta);

void rb_sm_icp();
void rb_sm_gpm();

void rb_sm_cleanup();

void rb_sm_get_x(double *x,double*y,double*theta);


#include <sm.h>

extern struct sm_params rb_sm_params;
extern struct sm_result rb_sm_result;

#endif
