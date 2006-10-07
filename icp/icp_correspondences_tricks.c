#include <gsl/gsl_vector.h>
#include "icp.h"
#include "journal.h"
#include "math_utils.h"

#define DEBUG_SEARCH(a) ;

int compatible(struct sm_params*params, int i, int j) {
	if(!params->doAlphaTest) return 1;
	
	double theta0 = 0; // FIXME
	if((params->laser_sens.alpha_valid[i]==0) ||
		(params->laser_ref.alpha_valid[j]==0)) 
		return 1;
		
	double alpha_i = params->laser_sens.alpha[i];
	double alpha_j = params->laser_ref.alpha[j];
	double tolerance = deg2rad(params->doAlphaTest_thresholdDeg);
	
	double theta = angleDiff(alpha_j, alpha_i);
	if(fabs(angleDiff(theta,theta0))>tolerance+deg2rad(params->maxAngularCorrectionDeg)) {
		return 0;
	} else {
		return 1;
	}
}

void find_correspondences_tricks(struct sm_params*params, gsl_vector* x_old) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
	double maxDist = params->maxCorrespondenceDist;
	if(jf()) fprintf(jf(),"param maxCorrespondenceDist %f\n",params->maxCorrespondenceDist);
	if(jf()) fprintf(jf(),"param maxLinearCorrection %f\n",params->maxLinearCorrection);
	if(jf()) fprintf(jf(),"param maxAngularCorrectionDeg %f\n",params->maxAngularCorrectionDeg);
	
	gsl_vector * p_i_w = gsl_vector_alloc(3);

	int last_best = -1;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_ray(laser_sens,i)) {
			ld_set_null_correspondence(laser_sens,i);
			continue; 
		}

		transform(laser_sens->p[i], x_old, p_i_w);
		double p_i_w_nrm2 = norm(p_i_w);
		
		int j1 = -1;
		double best_dist = 42;
		
		int from; int to; int start_cell;
		possible_interval(p_i_w, laser_ref, params->maxAngularCorrectionDeg,
			params->maxLinearCorrection, &from, &to, &start_cell);

//		printf("> i=%d [from %d to %d]\n",	i,from,to);

		int we_start_at = (last_best!=-1) ? start_cell : last_best;
			 we_start_at = minmax(from, to, we_start_at);
		
		int up =  we_start_at+1; 
		int down = we_start_at; 
		double last_dist_up = -1; // first is up
		double last_dist_down = 0;	

		int up_stopped = 0; 
		int down_stopped = 0;
		int dbg_number_points = 0;
	
		DEBUG_SEARCH(printf("i=%d p_i_w = %f %f\n",i, gvg(p_i_w,0),gvg(p_i_w,1)));
		DEBUG_SEARCH(printf("i=%d [from %d down %d mid %d up %d to %d]\n",
			i,from,down,start_cell,up,to));
			
		while ( (!up_stopped) || (!down_stopped) ) {
			int now_up = up_stopped ? 0 : 
			           down_stopped ? 1  : last_dist_up < last_dist_down;
			DEBUG_SEARCH(printf("|"));
			if(now_up) {
				DEBUG_SEARCH(printf("up %d ",up));
				if(up > to) { up_stopped = 1; continue; }
				if(!ld_valid_ray(laser_ref,up)) { ++up; continue; }
				++dbg_number_points;
				last_dist_up = distance(p_i_w, laser_ref->p[up]);
				if( (last_dist_up<maxDist) && ((j1==-1)||(last_dist_up < best_dist))) {
					if(compatible(params, i, up)) {
						j1 = up; best_dist = last_dist_up;
					}
				}
				double delta_theta = GSL_MAX(up-start_cell,0) * (M_PI/laser_ref->nrays);
				double min_dist_up = sin(delta_theta) * p_i_w_nrm2;
				if(min_dist_up > best_dist){ up_stopped = 1; continue;}
				
				if (up>start_cell) {
					up += (laser_ref->readings[up] <= p_i_w_nrm2) ?
						laser_ref->up_bigger[up] : laser_ref->up_smaller[up];
				} else ++up;
			}
			
			if(!now_up) {
				DEBUG_SEARCH(printf("down %d ",down));
				if(down < from) { down_stopped = 1; continue; }
				if(!ld_valid_ray(laser_ref,down)) { --down; continue; }
		
				++dbg_number_points;
				last_dist_down = distance(p_i_w, laser_ref->p[down]);
				if( (last_dist_down<maxDist) && ((j1==-1)||(last_dist_down < best_dist))) {
					if(compatible(params, i, down)) {
						j1 = down; best_dist = last_dist_down;
					}
				}

				double delta_theta = GSL_MAX(start_cell-down,0) * (M_PI/laser_ref->nrays);
				double min_dist_down = sin(delta_theta) * p_i_w_nrm2;
				if(min_dist_down > best_dist){ down_stopped = 1; continue;}
	
				if (down<start_cell) {
					down += (laser_ref->readings[down] <= p_i_w_nrm2) ?
						laser_ref->down_bigger[down] : laser_ref->down_smaller[down];
				} else --down;
			}
			
		}
		
		DEBUG_SEARCH(printf("i=%d j1=%d dist=%f\n",i,j1,best_dist));
		
		if(j1==-1) {// no match
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		// Do not match with extrema
		if( (j1==0) || (j1 == (laser_ref->nrays-1))) {// no match
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}

		int j2;
		int j2up   = ld_next_valid_up   (laser_ref, j1);
		int j2down = ld_next_valid_down (laser_ref, j1);
		if((j2up==-1)&&(j2down==-1)) {
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		if(j2up  ==-1) { j2 = j2down; } else
		if(j2down==-1) { j2 = j2up; } else {
			double dist_up   = distance(p_i_w, laser_ref->p[j2up  ]);
			double dist_down = distance(p_i_w, laser_ref->p[j2down]);
			j2 = dist_up < dist_down ? j2up : j2down;
		}

		last_best = j1;
		ld_set_correspondence(laser_sens, i, j1, j2);
	}
	
}



