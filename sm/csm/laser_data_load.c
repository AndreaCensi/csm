#include "csm_all.h"
#include "laser_data_drawing.h"

/** Loads all laser data */

int ld_read_some(FILE*file, LDP **array, int*num, int (*accept)(LDP)) {
	*array = 0; *num = 0;
	int size = 10;
	LDP * ar = (LDP*) malloc(sizeof(LDP)*size);
	
	while(1) {
		LDP ld = ld_read_smart(file);
		if(!ld) break;
		
		if( ! (*accept)(ld) ) {
			ld_free(ld);
			continue;
		}
		
		ar[(*num)++] = ld;
		
		if(*num > size - 1) {
			size *= 2;
			if(! (ar = (LDP*) realloc(ar, sizeof(LDP)*size)) ) {
				sm_error("Cannot allocate (size=%d)\n", size);
				return 0;
			}
		}
	}

	*array = ar; 

	return feof(file);
}


/* Read every tot scans */
int interval_count = 0;
int interval_interval = 10;
int interval_accept(LDP ld) {
	ld=ld;
	int accept = interval_count % interval_interval == 0;
	interval_count++;
	return accept;
}

int ld_read_some_scans(FILE*file, LDP **array, int*num, int interval) {
	interval_count = 0;
	interval_interval = interval;
	return ld_read_some(file, array, num, interval_accept);
}

/* Read all scans */
int always(LDP ld)  { ld=ld; return 1; }
int ld_read_all(FILE*file, LDP **array, int*num) {
	return ld_read_some(file, array, num, always);
}

/* Read according to distance */


static int distance_count;
static double distance_last_pose[3];
static double distance_interval_xy = 10;
static double distance_interval_th = 10;
static ld_reference distance_reference;

void distance_accept_reset(ld_reference which, double interval_xy, double interval_th) {
	distance_count = 0;
	distance_interval_xy = interval_xy;
	distance_interval_th = interval_th;
	distance_reference = which;
}

int distance_accept(LDP ld) {
	double * this_pose = ld_get_reference_pose(ld, distance_reference);
	if(!this_pose) return 0;
	
	distance_count++;
	if(distance_count == 1) {
		copy_d(this_pose, 3, distance_last_pose);
		return 1;
	} else {
		double diff[3];
		pose_diff_d(distance_last_pose, this_pose, diff);
		double distance  = norm_d(diff);
		
		if(distance >= distance_interval_xy || 
		   fabs(diff[2]) >= distance_interval_th ) 
		{
			copy_d(this_pose, 3, distance_last_pose);
		/*	sm_debug("Accepting #%d, %f\n", distance_count, distance);*/
			return 1;
		}
		else 
			return 0;
	}
}

int ld_read_some_scans_distance(FILE*file, LDP **array, int*num, 
	ld_reference which, double d_xy, double d_th) {
	distance_accept_reset(which, d_xy, d_th);
	return ld_read_some(file, array, num, distance_accept);
}



