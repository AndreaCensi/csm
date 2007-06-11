#ifndef H_LASER_DATA
#define H_LASER_DATA

#include <stdio.h>

#ifndef RUBY
#include <egsl/egsl.h>
#endif

struct correspondence;

typedef struct {
	double p[2];
	double rho, phi;
} point2d;


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

	double *cov_readings;

	double *true_alpha;
	
	/* Jump tables needed by find_correspondences_tricks(). */
	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;
	
	struct correspondence* corr;

	double true_pose[3];		
	double odometry[3];	
	double estimate[3];	
	
#ifndef RUBY
	/* Cartesian points */
/*	gsl_vector**p;*/
#endif

	/** Cartesian representation */
	point2d * points;
	/** Cartesian representation, in "world" (laser_ref) coordinates. */
	point2d * points_w;

};

struct correspondence {
	int valid; 
	/** Closest point in the other scan.  */
	int j1; 
	/** Second closest point in the other scan.  */
	int j2;
	/** Squared distance from p(i) to point j1 */
	double dist2_j1; 
};

typedef struct laser_data* LDP;

/** This returns a new structure, with all fields initialized */
LDP ld_alloc_new(int nrays);

/** This DOES free()s the pointer  */
void ld_free(LDP);

/** This allocs the fields in the given structure. Use ld_alloc_new(), not this. */
void ld_alloc(LDP, int nrays);

/** This does NOT free the pointer. Don't use -- use ld_alloc_new()/ld_free() instead. */
void ld_dealloc(LDP);

/** Fills the "points" field */
void ld_compute_cartesian(LDP);

/** Computes the "points_w" coordinates by roto-translating "points" */
void ld_compute_world_coords(LDP, const double *pose);

/** Fills the fields: *up_bigger, *up_smaller, *down_bigger, *down_smaller.*/
void ld_create_jump_tables(LDP);

/** Computes an hash of the correspondences */
unsigned int ld_corr_hash(LDP);

/** -1 if not found */
int ld_next_valid(LDP ld, int i, int dir);

/** True if the i-th is a valid ray */
int ld_valid_ray(LDP ld, int i);

/** True if the i-th is a valid correspondences */
int ld_valid_corr(LDP ld, int i);

/** Returns the number of valid correspondences. */
int ld_num_valid_correspondences(LDP);

/** Sets the i-th correspondence as valid. */
void ld_set_correspondence(LDP, int i, int j1, int j2);

/** Marks the i-th correspondence as invalid. */
void ld_set_null_correspondence(LDP, int i);

/** Find the next valid ray (j > i), or -1 if not found. */
int ld_next_valid_up(LDP, int i);

/** Find the prev valid ray (j < i), or -1 if not found.*/
int ld_next_valid_down(LDP, int i);

/** Do an extensive sanity check about the data contained in the structure. */
int ld_valid_fields(LDP);

/** A simple clustering algorithm. Sets the `cluster' field in the structure. */
void ld_simple_clustering(LDP ld, double threshold);

/** A cool orientation estimation algorithm. Needs cluster. */
void ld_compute_orientation(LDP ld, int size_neighbourhood, double sigma);

/** Read next FLASER line in file (initializes ld). 
	Returns 0 on success, -1 if error, -2 eof. 
	You probably want to use the ld_read_smart() function. */
int ld_read_next_laser_carmen(FILE*, LDP ld);

/** Reads all the scans it can find. */
int ld_read_all(FILE*file, LDP **array, int*num);

#endif

