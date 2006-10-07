#include <stdio.h>

#include "rb_sm.h"
#include <sm.h>

struct sm_params rb_sm_params; 
struct sm_result rb_sm_result;

void rb_sm_init_journal(const char*journal_file){
	sm_journal_open(journal_file);
}

struct laser_data * get_ld(int index) {
	return index==0 ? &(rb_sm_params.laser_ref) : &(rb_sm_params.laser_sens); 
}

void rb_sm_l_nrays(int laser, int nrays){
	struct laser_data * ld = get_ld(laser);
	ld_alloc(ld, nrays);	
}

void rb_sm_l_min_theta(int laser, double min_theta){
	get_ld(laser)->min_theta = min_theta;
}

void rb_sm_l_max_theta(int laser, double max_theta){
	get_ld(laser)->max_theta = max_theta;	
}

void rb_sm_l_ray(int laser, int ray, double theta, double reading){
	get_ld(laser)->readings[ray] = reading;
	get_ld(laser)->   theta[ray] = theta;
}

void rb_sm_odometry(double x, double y, double theta){
	rb_sm_params.odometry[0]=x;
	rb_sm_params.odometry[1]=y;
	rb_sm_params.odometry[2]=theta;
}

void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta){
	
	
}

void rb_sm_icp() {
	sm_icp(&rb_sm_params, &rb_sm_result);
}

void rb_sm_gpm() {
	sm_gpm(&rb_sm_params, &rb_sm_result);
}

void rb_sm_cleanup() {
	ld_free(&(rb_sm_params.laser_ref));
	ld_free(&(rb_sm_params.laser_sens));
}

void rb_sm_get_x(double *x,double*y,double*theta) {
	*x = rb_sm_result.x[0];
	*y = rb_sm_result.x[1];
	*theta = rb_sm_result.x[2];
}
