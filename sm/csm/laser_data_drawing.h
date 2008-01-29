#ifndef H_LASER_DATA_DRAWING
#define H_LASER_DATA_DRAWING

#include "laser_data.h"

typedef enum { Invalid = 0, Odometry = 1, Estimate = 2, True_pose = 3 } ld_reference;

const char*ld_reference_to_string(ld_reference);
ld_reference ld_string_to_reference(const char*string);

/** Gets a pointer to the pose specified by 'use_reference',
    or 0 if use_reference is invalid. */ 
double * ld_get_reference_pose_silent(LDP ld, ld_reference use_reference);
/** Same as ld_get_reference_pose_silent(), but it also checks 
    that the pose is valid (no NANs inside). */
double * ld_get_reference_pose(LDP ld, ld_reference use_reference);



int ld_read_some_scans_distance(FILE*file, LDP **array, int*num, 
	ld_reference which, double d_xy, double d_th);

typedef struct {
	/** lower left point */
	double pose[3]; 
	/** width, height; */
	double size[2]; 
} oriented_bbox;

typedef oriented_bbox* BB2;


/** Returns != 0 if enough points were found */
int ld_get_bounding_box(LDP ld, double bb_min[2], double bb_max[2],
	double pose[3], double horizon);
	
void lda_get_bounding_box(LDP *ld, int nld, double bb_min[2], double bb_max[2],
	double offset[3], ld_reference use_reference, double horizon);

/*void lda_get_bb2(LDP *ld, int nld, BB2 bb2, ld_reference use_reference, double horizon);*/


void oriented_bbox_compute_corners(const BB2,
	double ul[2], double ur[2], double ll[2], double lr[2]);

void ld_get_oriented_bbox(LDP ld, double horizon, BB2);

/* Simple API for finding bounding box */
struct bbfind_imp;
typedef struct bbfind_imp bbfind;
bbfind * bbfind_new();

int bbfind_add_point(bbfind*, double point[2]);
int bbfind_add_point2(bbfind*, double x, double y);
int bbfind_add_bbox(bbfind*, const BB2);

int bbfind_compute(bbfind*, BB2);
void bbfind_free(bbfind*);

/* For drawing stroke */

struct stroke_sequence {
	int begin_new_stroke;
	int end_stroke;
	int valid;
	
	double p[2];
}; 

void compute_stroke_sequence(LDP ld, struct stroke_sequence*,
	double horizon, double connect_threshold);


#endif
