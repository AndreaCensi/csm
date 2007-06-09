#include <gsl/gsl_vector.h>

#include "../csm_all.h"

#define DEBUG_SEARCH(a) ;


void ld_create_jump_tables(struct laser_data* ld) {
	int i;
	for(i=0;i<ld->nrays;i++) {
		int j;
		
		j=i+1;
		while(j<ld->nrays && ld->valid[j] && ld->readings[j]<=ld->readings[i]) j++;
		ld->up_bigger[i] = j-i;

		j = i+1;
		while(j<ld->nrays && ld->valid[j] && ld->readings[j]>=ld->readings[i]) j++;
		ld->up_smaller[i] = j-i;
		
		j = i-1;
		while(j>=0 && ld->valid[j] && ld->readings[j]>=ld->readings[i]) j--;
		ld->down_smaller[i] = j-i;

		j = i-1;
		while(j>=0 && ld->valid[j] && ld->readings[j]<=ld->readings[i]) j--;
		ld->down_bigger[i] = j-i;
	}	
}

extern int distance_counter;

#define INLINE

INLINE double local_distance_d(const double* a, const double* b) {
	distance_counter++;
	double x = a[0]-b[0];
	double y = a[1]-b[1];
	return sqrt(x*x+y*y);
}

INLINE double local_distance_squared_d(const double* a, const double* b) {
	distance_counter++;
	double x = a[0]-b[0];
	double y = a[1]-b[1];
	return x*x+y*y;
}

void find_correspondences_tricks(struct sm_params*params, gsl_vector* x_old) {
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	

	double pose_x = gvg(x_old, 0);
	double pose_y = gvg(x_old, 1);
	double pose_theta = gvg(x_old, 2);
	double cos_theta = cos(pose_theta); 
	double sin_theta = sin(pose_theta);

	double C0 = M_PI/laser_ref->nrays;
	double C1 =  (double)laser_ref->nrays / (laser_ref->max_theta-laser_ref->min_theta) ;
	
	int last_best = -1;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_ray(laser_sens,i)) {
			ld_set_null_correspondence(laser_sens,i);
			continue; 
		}

		double *p_i = laser_sens->p[i]->data;
		double p_i_w[2];
		
		p_i_w[0] = cos_theta * p_i[0] -sin_theta*p_i[1] + pose_x;
		p_i_w[1] = sin_theta * p_i[0] +cos_theta*p_i[1] + pose_y;
		
		double p_i_w_nrm2 = sqrt(square(p_i_w[0])+square(p_i_w[1]));
		
		
		int from; int to; int start_cell;
		
/*		if(0) {
		possible_interval(p_i_w, laser_ref, params->max_angular_correction_deg,
			params->max_linear_correction, &from, &to, &start_cell);
		} else {*/
			from = 0; to = laser_ref->nrays-1; 
			
			if(1) {
			/* To be turned into an interval of cells */
			double start_theta = atan2(p_i_w[1] , p_i_w[0]);

			start_cell  = (int)
				((start_theta - laser_ref->min_theta) * C1); /* C1 = 
				( 1 / (laser_ref->max_theta-laser_ref->min_theta) * laser_ref->nrays)); */
			
			} else {
				if(laser_sens->corr[i].valid)
					start_cell = laser_sens->corr[i].j1;
				else
					start_cell = i; /** XXX fai il min */
			}
/*		}*/

		int j1 = -1;
		double best_dist = 42;
		
/*		printf("> i=%d [from %d to %d]\n",	i,from,to); */

		int we_start_at = (last_best==-1) ? start_cell : last_best + 1;
			 we_start_at = minmax(from, to, we_start_at);
		
		int up =  we_start_at+1; 
		int down = we_start_at; 
		double last_dist_up = 0; /* first is down */
		double last_dist_down = -1;	

		int up_stopped = 0; 
		int down_stopped = 0;
	
		DEBUG_SEARCH(printf("i=%d p_i_w = %f %f\n",i, p_i_w[0], p_i_w,[1]));
		DEBUG_SEARCH(printf("i=%d [from %d down %d mid %d up %d to %d]\n",
			i,from,down,start_cell,up,to));
		
		
		while ( (!up_stopped) || (!down_stopped) ) {
			int now_up = up_stopped ? 0 : 
			           down_stopped ? 1 : last_dist_up < last_dist_down;
			DEBUG_SEARCH(printf("|"));

			/* Now two symmetric chunks of code, the now_up and the !now_up */
			if(now_up) {
				DEBUG_SEARCH(printf("up %d ",up));
				/* If we have crossed the "to" boundary we stop searching
					on the "up" direction. */
				if(up > to) { up_stopped = 1; continue; }
				/* Just ignore invalid rays */
				if(!laser_ref->valid[up]) { ++up; continue; }
				
				/* This is the distance from p_i_w to the "up" point*/
				last_dist_up = local_distance_squared_d(p_i_w, laser_ref->p[up]->data);
				
				/* If it is less than the best point, it is our new j1 */
				if( (last_dist_up<square(params->max_correspondence_dist)) && 
					((j1==-1)||(last_dist_up < best_dist))) {
						j1 = up; best_dist = last_dist_up;
				}
				
				/* If we are moving away from start_cell */
				if (up>start_cell) {
					/* We can compute a bound for early stopping. Currently
					   our best point has distance best_dist; we can compute
					   min_dist_up, which is the minimum distance that can have
					   points for j>up (see figure)*/
					double delta_theta = (up-start_cell) * C0; /*C0 = (M_PI/laser_ref->nrays);*/
					double min_dist_up = sin(delta_theta) * p_i_w_nrm2;
					/* If going up we can't make better than best_dist, then
					    we stop searching in the "up" direction */
					if(square(min_dist_up) > best_dist) { 
						up_stopped = 1; continue;
					}
					/* If we are moving away, then we can implement the jump tables
					   optimizations. */
					up += 
						/* If p_i_w is shorter than "up" */
						(laser_ref->readings[up] <= p_i_w_nrm2) 
						?
						/* We can jump to a bigger point */
						laser_ref->up_bigger[up] 
						/* Or else we jump to a smaller point */ 
						: laser_ref->up_smaller[up];
						
				} else 
					/* If we are moving towards "start_cell", we can't do any
					   ot the previous optimizations and we just move to the next point */
					++up;
				
			}
			
			/* This is the specular part of the previous chunk of code. */
			if(!now_up) {
				DEBUG_SEARCH(printf("down %d ",down));
				if(down < from) { down_stopped = 1; continue; }
				if(!laser_ref->valid[down]) { --down; continue; }
		
				last_dist_down = local_distance_squared_d(p_i_w, laser_ref->p[down]->data);
				if( (last_dist_down<square(params->max_correspondence_dist)) && 
				    ((j1==-1)||(last_dist_down < best_dist))) {
						j1 = down; best_dist = last_dist_down;
				}

				if (down<start_cell) {
				/*	double min_dist_down = table[start_cell-down] * p_i_w_nrm2;*/
					double delta_theta = (start_cell-down) * C0; /* C0 = (M_PI/laser_ref->nrays) */;
					double min_dist_down = sin(delta_theta) * p_i_w_nrm2;
					if( square(min_dist_down) > best_dist) { 
						down_stopped = 1; continue;
					}
					down += (laser_ref->readings[down] <= p_i_w_nrm2) ?
						laser_ref->down_bigger[down] : laser_ref->down_smaller[down];
				} else --down;
			}
			
		}
		
		DEBUG_SEARCH(printf("i=%d j1=%d dist=%f\n",i,j1,best_dist));
		
		/* If no point matched. */
		if(-1==j1) {
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		/* We ignore matching the first or the last point in the scan */
		if( 0==j1 || j1 == (laser_ref->nrays-1)) {/* no match */
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}

		/* Now we want to find j2, the second best match. */
		int j2;
		/* We find the next valid point, up and down */
		int j2up   = ld_next_valid_up   (laser_ref, j1);
		int j2down = ld_next_valid_down (laser_ref, j1);
		/* And then (very boring) we use the nearest */
		if((j2up==-1)&&(j2down==-1)) {
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		if(j2up  ==-1) { j2 = j2down; } else
		if(j2down==-1) { j2 = j2up; } else {
			double dist_up   = local_distance_squared_d(p_i_w, laser_ref->p[j2up  ]->data);
			double dist_down = local_distance_squared_d(p_i_w, laser_ref->p[j2down]->data);
			j2 = dist_up < dist_down ? j2up : j2down;
		}

		last_best = j1;
		ld_set_correspondence(laser_sens, i, j1, j2);
	}

}



