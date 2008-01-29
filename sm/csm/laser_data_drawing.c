#include <string.h>
#include <strings.h>
#include "laser_data_drawing.h"
#include "logging.h"
#include "math_utils.h"

const char*ld_reference_name[4] = { "invalid","odometry","estimate","true_pose"};

const char*ld_reference_to_string(ld_reference r) {
	return ld_reference_name[r];
}

ld_reference ld_string_to_reference(const char*s) {
	int i; for(i=1;i<=3;i++) 
		if(!strcasecmp(s, ld_reference_to_string( (ld_reference) i) ))
			return (ld_reference) i;
			
	sm_error("Could not translate string '%s' to a reference name.\n", s);
	return Invalid;
}

int ld_get_bounding_box(LDP ld, double bb_min[2], double bb_max[2],
	double pose[3], double horizon) {

	int rays_used = 0;
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld->valid[i]) continue;
		if(ld->readings[i]>horizon) continue;

		double p0[2] = {
			cos(ld->theta[i]) * ld->readings[i],
			sin(ld->theta[i]) * ld->readings[i]
		};
		
		double p[2];
		transform_d(p0,pose,p);
		
		if(0 == rays_used) {
			bb_min[0] = bb_max[0] = p[0];
			bb_min[1] = bb_max[1] = p[1];
		} else {
			int j=0; for(j=0;j<2;j++) {
				bb_min[j] = GSL_MIN(bb_min[j], p[j]);
				bb_max[j] = GSL_MAX(bb_max[j], p[j]);
			}
		}

		rays_used++;
	}
		
	return rays_used > 3;
}

/*void lda_get_bb2(LDP *ld, int nld, BB2 bb2, ld_reference use_reference, double horizon) {
	double bb_min[2], bb_max[2], offset[3] = {0,0,0};
	lda_get_bounding_box(ld,nld, bb2->bb_)
}*/

	
void lda_get_bounding_box(LDP *lda, int nld, double bb_min[2], double bb_max[2],
	double offset[3], ld_reference use_reference, double horizon) {
	
	int k;
	for(k=0;k<nld;k++) {
		LDP ld = lda[k];

		double *ref = ld_get_reference_pose(ld, use_reference);
		if(!ref) {
			sm_error("Pose %s not set in scan #%d.\n", 
				ld_reference_to_string(use_reference), k);
			continue;
		}
		
		double pose[3];
		oplus_d(offset, ref, pose);
	
		if(k==0) 
			ld_get_bounding_box(ld, bb_min, bb_max, pose, horizon);
		else {
			double this_min[2], this_max[2];
			ld_get_bounding_box(ld, this_min, this_max, pose, horizon);
			int i; for(i=0;i<2;i++) {
				bb_min[i] = GSL_MIN(bb_min[i], this_min[i]);
				bb_max[i] = GSL_MAX(bb_max[i], this_max[i]);
			}
		}
	}
}

double * ld_get_reference_pose_silent(LDP ld, ld_reference use_reference) {
	double * pose;
	switch(use_reference) {
		case Odometry: pose = ld->odometry; break;
		case Estimate: pose = ld->estimate; break;
		case True_pose: pose = ld->true_pose; break;
		default: 
			sm_error("Could not find pose identified by %d.\n", (int) use_reference);
			return 0;
	}
	return pose;
}

double * ld_get_reference_pose(LDP ld, ld_reference use_reference) {
	double * pose = ld_get_reference_pose_silent(ld, use_reference);
	if(any_nan(pose, 3)) {
		sm_error("Required field '%s' not set in laser scan.\n", 
			ld_reference_to_string(use_reference) );
		return 0;
	}
	return pose;
}


void compute_stroke_sequence(LDP ld, struct stroke_sequence*draw_info,
	double horizon, double connect_threshold) {
	int last_valid = -1; int first = 1;
	int i; for(i=0;i<ld->nrays;i++) {
		if( (!ld_valid_ray(ld,i)) || (ld->readings[i] > horizon) ) {
			draw_info[i].valid = 0;
			continue;
		}
		draw_info[i].valid = 1;
		draw_info[i].p[0] = ld->readings[i] * cos(ld->theta[i]);
		draw_info[i].p[1] = ld->readings[i] * sin(ld->theta[i]);
		
		if(first) { 
			first = 0; 
			draw_info[i].begin_new_stroke = 1;
			draw_info[i].end_stroke = 0;
		} else {
			int near =  square(connect_threshold) > 
				distance_squared_d(draw_info[last_valid].p, draw_info[i].p);
			draw_info[i].begin_new_stroke = near ? 0 : 1;
			draw_info[i].end_stroke = 0;
			draw_info[last_valid].end_stroke = draw_info[i].begin_new_stroke;
		}
		last_valid = i;
	} /*for */
	if(last_valid >= 0)
		draw_info[last_valid].end_stroke = 1;
} /* find buff .. */



