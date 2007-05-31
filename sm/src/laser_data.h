#ifndef H_LASER_DATA
#define H_LASER_DATA

#include <stdio.h>

#ifndef RUBY
#include <egsl.h>
#endif

struct correspondence;

struct laser_data {
	int nrays;
	double  min_theta;
	double  max_theta;
	
	double *theta;
	
	int    *valid;
	double *readings;
	
	
	int    *cluster;
	
	double *alpha;
	double *cov_alpha;
	int    *alpha_valid;

	double *true_alpha;
	
	/* Jump tables */
	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;
	
	struct correspondence* corr;

	double true_pose[3];		
	double odometry[3];	
	double estimate[3];	
	
#ifndef RUBY
	/* Cartesian points */
	gsl_vector**p;
#endif

};

struct correspondence {
	int valid; 
	int j1; int j2;
};

typedef struct laser_data* LDP;

LDP ld_alloc_new(int nrays);
/** This DOES free()s the pointer  */
void ld_free(LDP);

void ld_alloc(LDP, int nrays);
/** This does NOT free the pointer */
void ld_dealloc(LDP);

void ld_compute_cartesian(LDP);
void ld_create_jump_tables(LDP);

/** -1 if not found */
int ld_next_valid(LDP ld, int i, int dir);

int ld_valid_ray(LDP ld, int i);
int ld_valid_corr(LDP ld, int i);

int ld_num_valid_correspondences(LDP ld);

void ld_set_correspondence(LDP, int i, int j1, int j2);
void ld_set_null_correspondence(LDP, int i);
/** -1 if not found */
int ld_next_valid_up(LDP, int i);
int ld_next_valid_down(LDP, int i);


void ld_simple_clustering(LDP ld, double threshold);
void ld_compute_orientation(LDP ld, int size_neighbourhood, double sigma);

/** Read next FLASER line in file (initializes ld). 
	Returns 0 on success, -1 if error, -2 eof. */
int ld_read_next_laser_carmen(FILE*, LDP ld);

#endif

