#include <math.h>
#include "laser_data.h"
#include "math_utils.h"
#include "journal.h"

void ld_alloc(struct laser_data*ld, int nrays) {
	ld->nrays = nrays;
	ld->readings     = (double*) malloc(sizeof(double)*nrays);
	ld->theta        = (double*) malloc(sizeof(double)*nrays);
	
	ld->cluster = (int*) malloc(sizeof(int)   *nrays);
	ld->alpha = (double*) malloc(sizeof(double)*nrays);
	ld->cov_alpha = (double*) malloc(sizeof(double)*nrays);
	ld->alpha_valid = (int*) malloc(sizeof(int)   *nrays);
	
	int i;
	for(i=0;i<ld->nrays;i++) {
		ld->theta[i] = GSL_NAN;
		ld->readings[i] = GSL_NAN;
		ld->cluster[i] = -1;
		ld->alpha[i] = GSL_NAN;
		ld->cov_alpha[i] = GSL_NAN;
		ld->alpha_valid[i] = 0;
	}
	
	ld->up_bigger    =    (int*) malloc(sizeof(int)   *nrays);
	ld->up_smaller   =    (int*) malloc(sizeof(int)   *nrays);
	ld->down_bigger  =    (int*) malloc(sizeof(int)   *nrays);
	ld->down_smaller =    (int*) malloc(sizeof(int)   *nrays);
	ld->corr         = 
		(struct correspondence*) malloc(sizeof(struct correspondence)*nrays);
	
	ld->p = (gsl_vector**) malloc(sizeof(gsl_vector*) * nrays);

	for(i=0;i<nrays;i++) {
		ld->p[i] = gsl_vector_alloc(2);
		gvs(ld->p[i], 0, GSL_NAN);
		gvs(ld->p[i], 1, GSL_NAN);
	}
}

void ld_free(struct laser_data*ld){	
	free(ld->readings);
	free(ld->theta);
	free(ld->cluster);
	free(ld->alpha);
	free(ld->alpha_valid);
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

int ld_valid_ray(struct laser_data* ld, int i) {
	return (i>=0) && (i<ld->nrays) && (ld->readings[i] > 0);
}

int ld_valid_corr(LDP ld, int i) {
	return ld->corr[i].valid;
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

// -1 if not found

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

void ld_create_jump_tables(struct laser_data* ld) {
	int i;
	for(i=0;i<ld->nrays;i++) {
		int j=i+1;

		while(ld_valid_ray(ld,j) && ld->readings[j]<=ld->readings[i]) j++;
		ld->up_bigger[i] = j-i;

		j = i+1;
		while(ld_valid_ray(ld,j) && ld->readings[j]>=ld->readings[i]) j++;
		ld->up_smaller[i] = j-i;
		
		j = i-1;
		while(ld_valid_ray(ld,j) && ld->readings[j]>=ld->readings[i]) j--;
		ld->down_smaller[i] = j-i;

		j = i-1;
		while(ld_valid_ray(ld,j) && ld->readings[j]<=ld->readings[i]) j--;
		ld->down_bigger[i] = j-i;
	}
	
	journal_write_array_i("down_bigger", ld->nrays, ld->down_bigger );
	journal_write_array_i("down_smaller",ld->nrays, ld->down_smaller);
	journal_write_array_i("up_bigger",   ld->nrays, ld->up_bigger );
	journal_write_array_i("up_smaller",  ld->nrays, ld->up_smaller);	
}
