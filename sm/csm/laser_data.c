#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "csm_all.h"


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
	ld->cov_readings = alloc_double_array(nrays, GSL_NAN);
	ld->theta        = alloc_double_array(nrays, GSL_NAN);
	
	ld->min_theta = GSL_NAN;
	ld->max_theta = GSL_NAN;
	
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
		ld->corr[i].j1 = -1;
		ld->corr[i].j2 = -1;
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
	free(ld->cov_readings);
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
	ld->corr[i].j1 = -1;	
	ld->corr[i].j2 = -1;	
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
		} else {
			gsl_vector_set(ld->p[i], 0, cos(ld->theta[i])*ld->readings[i]);
			gsl_vector_set(ld->p[i], 1, sin(ld->theta[i])*ld->readings[i]);
		}
	}
}

/** -1 if not found */

int ld_next_valid(LDP ld, int i, int dir) {
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

int count_equal(const int*v, int n, int value) {
	int num = 0, i;
	for(i=0;i<n;i++) if(value == v[i]) num++;
	return num;
}

int ld_valid_fields(LDP ld)  {
	if(!ld) {
		sm_error("NULL pointer given as laser_data*.\n");	
		return 0;
	}
	
	int min_nrays = 10;
	int max_nrays = 10000;
	if(ld->nrays < min_nrays || ld->nrays > max_nrays) {
		sm_error("Invalid number of rays: %d\n", ld->nrays);
		return 0;
	}
	if(is_nan(ld->min_theta) || is_nan(ld->max_theta)) {
		sm_error("Invalid min / max theta: min_theta = %f max_theta = %f\n",
			ld->min_theta, ld->max_theta);
		return 0;
	}
	double min_fov = deg2rad(20.0); 
	double max_fov = 2 * M_PI;
	double fov = ld->max_theta - ld->min_theta;
	if( fov < min_fov || fov > max_fov) {
		sm_error("Strange FOV: %f rad = %f deg \n", fov, rad2deg(fov));
		return 0;
	}
	if(ld->min_theta != ld->theta[0]) {
		sm_error("Min_theta (%f) should be theta[0] (%f)\n",
			ld->min_theta, ld->theta[0]);
		return 0;
	}
	if(ld->max_theta != ld->theta[ld->nrays-1]) {
		sm_error("Min_theta (%f) should be theta[0] (%f)\n",
			ld->max_theta, ld->theta[ld->nrays-1]);
		return 0;
	}
	/* Check that there are valid rays */
	double min_reading = 0;
	double max_reading = 100;
	int i; for(i=0;i<ld->nrays;i++) {
		if(ld->valid[i]) {
			double r = ld->readings[i];
			double th = ld->theta[i];
			if(is_nan(r) || is_nan(th)) {
				sm_error("Ray #%d: r = %f  theta = %f but valid is %d\n",
					i, r, th, ld->valid[i]);
				return 0;
			}
			if( !( min_reading < r && r < max_reading ) ) {
				sm_error("Ray #%d: %f is not in interval (%f, %f) \n",
					i, r, min_reading, max_reading);
				return 0;
			}
			
		}
	}
	/* Checks that there is at least 10% valid rays */
	int num_valid   = count_equal(ld->valid, ld->nrays, 1);
	int num_invalid = count_equal(ld->valid, ld->nrays, 0);
	if (num_valid < ld->nrays * 0.10) {
		sm_error("Valid: %d/%d invalid: %d.\n", num_valid, ld->nrays, num_invalid);
		return 0;
	}

	return 1;
}


/** Computes an hash of the correspondences */
unsigned int ld_corr_hash(LDP ld){
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < (unsigned)ld->nrays; i++) {
		int str = ld_valid_corr(ld, (int)i) ? (ld->corr[i].j1 + 1000*ld->corr[i].j2) : -1;
		hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (str) ^ (hash >> 3)) :
		                         (~((hash << 11) ^ (str) ^ (hash >> 5)));
	}

	return (hash & 0x7FFFFFFF);
}
