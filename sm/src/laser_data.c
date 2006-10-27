#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "laser_data.h"
#include "math_utils.h"
#include "journal.h"

void ld_alloc(struct laser_data*ld, int nrays) {
	ld->nrays = nrays;
	ld->valid = (int*) malloc(sizeof(int)*nrays);
	ld->readings     = (double*) malloc(sizeof(double)*nrays);
	ld->theta        = (double*) malloc(sizeof(double)*nrays);
	
	ld->cluster = (int*) malloc(sizeof(int)   *nrays);
	ld->alpha = (double*) malloc(sizeof(double)*nrays);
	ld->cov_alpha = (double*) malloc(sizeof(double)*nrays);
	ld->alpha_valid = (int*) malloc(sizeof(int)   *nrays);
	
	ld->up_bigger    =    (int*) malloc(sizeof(int)   *nrays);
	ld->up_smaller   =    (int*) malloc(sizeof(int)   *nrays);
	ld->down_bigger  =    (int*) malloc(sizeof(int)   *nrays);
	ld->down_smaller =    (int*) malloc(sizeof(int)   *nrays);
	ld->corr         = 
		(struct correspondence*) malloc(sizeof(struct correspondence)*nrays);
	
	ld->p = (gsl_vector**) malloc(sizeof(gsl_vector*) * nrays);

	int i;
	for(i=0;i<nrays;i++) {
		ld->p[i] = gsl_vector_alloc(2);
		gvs(ld->p[i], 0, GSL_NAN);
		gvs(ld->p[i], 1, GSL_NAN);
	}

	for(i=0;i<ld->nrays;i++) {
		ld->valid[i] = 0;
		ld->theta[i] = GSL_NAN;
		ld->readings[i] = GSL_NAN;
		ld->cluster[i] = -1;
		ld->alpha[i] = GSL_NAN;
		ld->cov_alpha[i] = GSL_NAN;
		ld->alpha_valid[i] = 0;
		ld->corr[i].valid = 0;
		ld->corr[i].j1 = 0;
		ld->corr[i].j2 = 0;
	}
	
}

void ld_free(struct laser_data*ld){	
	free(ld->valid);
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



const char * prefix = "FLASER ";

// Returns 0 on success
int read_next_double(const char*line, int*cur, double*d) {
	int inc;
	if(1 != sscanf(line+*cur, " %lf %n", d, &inc)) {
		printf("Could not read double.\n");
		return -1;
	}
	*cur += inc;
	return 0;
}

int ld_valid_ray(struct laser_data* ld, int i) {
	return (i>=0) && (i<ld->nrays) && (ld->valid[i]);
}

int ld_valid_corr(LDP ld, int i) {
	return ld->corr[i].valid;
}


// Read next FLASER line in file (initializes ld). Returns 0 if error or eof.
int ld_read_next_laser_carmen(FILE*file, LDP ld) {
	#define MAX_LINE_LENGTH 10000
   char line[MAX_LINE_LENGTH];

	while(fgets(line, MAX_LINE_LENGTH-1, file)) {
		
		if(0 != strncmp(line, prefix, strlen(prefix))) {
			printf("Skipping line: \n-> %s\n", line);
			continue;
		}
		
		int cur = strlen(prefix); int inc;
		
		int nrays;
		if(1 != sscanf(line+cur, "%d %n", &nrays, &inc)) 
			goto error;
		cur += inc;
			
		ld_alloc(ld, nrays);	
		
		ld->min_theta = -M_PI/2;
		ld->max_theta = +M_PI/2;
		
		int i;
		for(i=0;i<nrays;i++) {
			double reading;
			if(read_next_double(line,&cur,&reading)) {
				printf("At ray #%d, ",i); 
				goto error;
			}
				
			ld->valid[i] = reading>0 && reading<80;
			ld->readings[i] = ld->valid[i] ? reading : GSL_NAN;
			ld->theta[i] = ld->min_theta+ i * (ld->max_theta-ld->min_theta) / (ld->nrays-1);
		}
		
		if(read_next_double(line,&cur,ld->odometry+0)) goto error;
		if(read_next_double(line,&cur,ld->odometry+1)) goto error;
		if(read_next_double(line,&cur,ld->odometry+2)) goto error;
		if(read_next_double(line,&cur,ld->estimate+0)) goto error;
		if(read_next_double(line,&cur,ld->estimate+1)) goto error;
		if(read_next_double(line,&cur,ld->estimate+2)) goto error;
		
		fprintf(stderr, "l");
		return 0;
		
		error:
			printf("Malformed line? \n-> %s\nat cur = %d\n\t-> %s\n", line,cur,line+cur);
			return -1;
	}
	return -2;
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


















