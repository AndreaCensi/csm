#ifndef H_SM_RUBY
#define H_SM_RUBY


/** Interface for Ruby: there are no pointers around. */

void rb_sm_init_journal(const char*journal_file);

void rb_sm_odometry(double x, double y, double theta);
void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta);

int rb_sm_icp();
int rb_sm_gpm();

const char *rb_result_to_json();

void rb_sm_get_x(double *x,double*y,double*theta);

void rb_sm_cleanup();


#include <csm/csm.h>

extern struct sm_params rb_sm_params;
extern struct sm_result rb_sm_result;

#endif
