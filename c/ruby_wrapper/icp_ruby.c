#include <stdio.h>

#include "../icp.c"
#include "../icp_loop.c"

struct icp_input  icpc_params;
struct icp_output icpc_res;

void icpc_init_journal(const char*journal_file){
	printf("Complimenti! inizializzato!\n");

}

struct laser_data * get_ld(int index) {
	return index ? &(icpc_params.laser_ref) : &(icpc_params.laser_sens); 
}

void icpc_l_nrays(int laser, int nrays){
	struct laser_data * ld = get_ld(laser);
	ld->nrays = nrays;
	ld->readings = (double*) malloc(sizeof(double)*nrays);
	ld->theta    = (double*) malloc(sizeof(double)*nrays);
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
	
}

void icpc_odometry_cov(double cov_x, double cov_y, double cov_theta){
	
	
}


void icpc_go() {
}

void icpc_cleanup() {
	free(icpc_params.laser_ref .readings);
	free(icpc_params.laser_ref .theta);
	free(icpc_params.laser_sens.readings);
	free(icpc_params.laser_sens.theta);
}