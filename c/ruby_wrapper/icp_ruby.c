
#include <stdio.h>
#include "icp_ruby.h"
#include "icp.h"

struct icp_input  icpc_params;
struct icp_output icpc_res;

void icpc_init_journal(const char*journal_file){
	icp_journal_open(journal_file);
}

struct laser_data * get_ld(int index) {
	return index==0 ? &(icpc_params.laser_ref) : &(icpc_params.laser_sens); 
}

void icpc_l_nrays(int laser, int nrays){
	struct laser_data * ld = get_ld(laser);
	ld_alloc(ld, nrays);	
}

void icpc_l_min_theta(int laser, double min_theta){
	get_ld(laser)->min_theta = min_theta;
}

void icpc_l_max_theta(int laser, double max_theta){
	get_ld(laser)->max_theta = max_theta;	
}

void icpc_l_ray(int laser, int ray, double theta, double reading){
	get_ld(laser)->readings[ray] = reading;
	get_ld(laser)->   theta[ray] = theta;
}

void icpc_odometry(double x, double y, double theta){
	icpc_params.odometry[0]=x;
	icpc_params.odometry[1]=y;
	icpc_params.odometry[2]=theta;
}

void icpc_odometry_cov(double cov_x, double cov_y, double cov_theta){
	
	
}

void icpc_go() {
	icp(&icpc_params, &icpc_res);
}

void icpc_cleanup() {
	ld_free(&(icpc_params.laser_ref));
	ld_free(&(icpc_params.laser_sens));
}

