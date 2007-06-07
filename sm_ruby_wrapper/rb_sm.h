#ifndef H_SM_RUBY_WRAPPER
#define H_SM_RUBY_WRAPPER

#include <csm/csm.h>

/** Interface for Ruby: there are no pointers around. */

int rb_sm_set_configuration(const char*name, const char*value);


void rb_sm_odometry(double x, double y, double theta);
void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta);

int rb_sm_icp();
int rb_sm_gpm();

const char *rb_result_to_json();

LDP string_to_ld(const char*s);


void rb_set_laser_ref(const char*s);
void rb_set_laser_sens(const char*s);

void rb_sm_cleanup();



void rb_sm_init_journal(const char*journal_file);
void rb_sm_close_journal();

#include <csm/csm.h>

extern struct sm_params rb_sm_params;
extern struct sm_result rb_sm_result;

#endif
