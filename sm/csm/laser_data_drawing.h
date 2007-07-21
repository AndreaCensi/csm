#ifndef H_LASER_DATA_DRAWING
#define H_LASER_DATA_DRAWING

#include "csm_all.h"

typedef enum { Invalid = 0, Odometry = 1, Estimate = 2, True_pose = 3 } ld_reference;

const char*ld_reference_to_string(ld_reference);

/** Returns 0 if nan */
double * ld_get_reference_pose(LDP ld, ld_reference use_reference);

/** Returns != 0 if enough points were found */
int ld_get_bounding_box(LDP ld, double bb_min[2], double bb_max[2],
	double pose[3], double horizon);
	
void lda_get_bounding_box(LDP *ld, int nld, double bb_min[2], double bb_max[2],
	double offset[3], ld_reference use_reference, double horizon);


int ld_read_some_scans_distance(FILE*file, LDP **array, int*num, 
	ld_reference which, double d_xy, double d_th);


#endif
