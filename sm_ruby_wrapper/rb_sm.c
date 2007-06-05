#include <stdio.h>
#include <gsl/gsl_nan.h>
#include <options/options.h>
#include "rb_sm.h"

struct sm_params rb_sm_params; 
struct sm_result rb_sm_result;

void rb_sm_init_journal(const char*journal_file){
	sm_journal_open(journal_file);
}

void rb_sm_odometry(double x, double y, double theta){
	rb_sm_params.first_guess[0]=x;
	rb_sm_params.first_guess[1]=y;
	rb_sm_params.first_guess[2]=theta;
}

void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta){
	
}

struct option* ops = 0;

int rb_sm_set_configuration(const char*name, const char*value) {
	if(!ops) { 
		ops = options_allocate(30);
		sm_options(&rb_sm_params, ops);
	}
	
	if(!options_try_pair(ops, name, value)) {

		return 0;
	} else
	return 1;
}


const char *rb_result_to_json() {
	static char buf[5000];
	JO jo = result_to_json(&rb_sm_params, &rb_sm_result);
	strcpy(buf, jo_to_string(jo));
	jo_free(jo);
	return buf;
}

int rb_sm_icp() {
	sm_icp(&rb_sm_params, &rb_sm_result);
	return rb_sm_result.valid;
}

int rb_sm_gpm() {	
	sm_gpm(&rb_sm_params, &rb_sm_result);
	return rb_sm_result.valid;
}

void rb_sm_cleanup() {
/*	ld_free(&(rb_sm_params.laser_ref));
	ld_free(&(rb_sm_params.laser_sens));*/
}

