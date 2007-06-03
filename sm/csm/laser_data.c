#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "laser_data.h"
#include "math_utils.h"
#include "journal.h"

LDP ld_alloc_new(int nrays) {
	LDP ld = malloc(sizeof(struct laser_data));
	ld_alloc(ld, nrays);
	return ld;
}

double* alloc_double_array(int n, double def) {
	double *v = (double*) malloc(sizeof(double)*n);
	int i=0; for(i=0;i<n;i++) {
		v[i] = def;
	}
	return v;
}

int* alloc_int_array(int n, int def) {
	int *v = (int*) malloc(sizeof(int)*n);
	int i=0; for(i=0;i<n;i++) {
		v[i] = def;
	}
	return v;
}

void ld_alloc(LDP ld, int nrays) {
	ld->nrays = nrays;
	
	ld->valid        = alloc_int_array(nrays, 0);
	ld->readings     = alloc_double_array(nrays, GSL_NAN);
	ld->theta        = alloc_double_array(nrays, GSL_NAN);
	
	ld->cluster      = alloc_int_array(nrays, -1);
	ld->alpha        = alloc_double_array(nrays, GSL_NAN);
	ld->cov_alpha    = alloc_double_array(nrays, GSL_NAN);
	ld->alpha_valid  = alloc_int_array(nrays, 0);

	ld->true_alpha   = alloc_double_array(nrays, GSL_NAN);
	
	ld->up_bigger    = alloc_int_array(nrays, 0);
	ld->up_smaller   = alloc_int_array(nrays, 0);
	ld->down_bigger  = alloc_int_array(nrays, 0);
	ld->down_smaller = alloc_int_array(nrays, 0);
	
	ld->p = (gsl_vector**) malloc(sizeof(gsl_vector*) * nrays);

	int i;
	for(i=0;i<nrays;i++) {
		ld->p[i] = gsl_vector_alloc(2);
		gvs(ld->p[i], 0, GSL_NAN);
		gvs(ld->p[i], 1, GSL_NAN);
	}

	ld->corr = (struct correspondence*) 
		malloc(sizeof(struct correspondence)*nrays);

	for(i=0;i<ld->nrays;i++) {
		ld->corr[i].valid = 0;
		ld->corr[i].j1 = 0;
		ld->corr[i].j2 = 0;
	}
	
	for(i=0;i<3;i++) {
		ld->odometry[i] = 
		ld->estimate[i] = 
		ld->true_pose[i] = GSL_NAN;
	}
}

void ld_free(LDP ld) {
	ld_dealloc(ld);
	free(ld);
}

void ld_dealloc(struct laser_data*ld){	
	free(ld->valid);
	free(ld->readings);
	free(ld->theta);
	free(ld->cluster);
	free(ld->alpha);
	free(ld->alpha_valid);
	free(ld->true_alpha);
	free(ld->cov_alpha);
	free(ld->up_bigger);
	free(ld->up_smaller);
	free(ld->down_bigger);
	free(ld->down_smaller);
	free(ld->corr);
	
	int i;
	for(i=0;i<ld->nrays;i++)
		gsl_vector_free(ld->p[i]);
	free(ld->p);
}

void ld_set_null_correspondence(struct laser_data*ld, int i) {
	ld->corr[i].valid = 0;
}

void ld_set_correspondence(LDP ld, int i, int j1, int j2) {
	ld->corr[i].valid = 1;
	ld->corr[i].j1 = j1;	
	ld->corr[i].j2 = j2;	
}

void ld_compute_cartesian(LDP ld) {
	int i;
	for(i=0;i<ld->nrays;i++) {
		if(!ld_valid_ray(ld,i)){
			gsl_vector_set_nan(ld->p[i]);
		}
		gsl_vector_set(ld->p[i], 0, cos(ld->theta[i])*ld->readings[i]);
		gsl_vector_set(ld->p[i], 1, sin(ld->theta[i])*ld->readings[i]);

	}
}

/** -1 if not found */

int ld_next_valid(LDP ld, int i, int dir){
	int j;
	for(j=i+dir;(j<ld->nrays)&&(j>=0)&&!ld_valid_ray(ld,j);j+=dir);
	return ld_valid_ray(ld,j) ? j : -1;
}

int ld_next_valid_up(LDP ld, int i){
	return ld_next_valid(ld, i, +1);
}

int ld_next_valid_down(LDP ld, int i){
	return ld_next_valid(ld, i, -1);
}




int ld_valid_ray(struct laser_data* ld, int i) {
	return (i>=0) && (i<ld->nrays) && (ld->valid[i]);
}

int ld_valid_corr(LDP ld, int i) {
	return ld->corr[i].valid;
}

int ld_num_valid_correspondences(LDP ld) {
	int i; 
	int num = 0;
	for(i=0;i<ld->nrays;i++) {
		if(ld->corr[i].valid)
			num++;
	}
	return num;
}
