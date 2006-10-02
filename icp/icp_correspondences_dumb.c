#include <gsl/gsl_vector.h>
#include "icp.h"
#include "journal.h"
#include "math_utils.h"

void find_correspondences(struct icp_input*params, gsl_vector* x_old) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);

	fprintf(jf(),"param maxCorrespondenceDist %f\n",params->maxCorrespondenceDist);
	fprintf(jf(),"param maxLinearCorrection %f\n",params->maxLinearCorrection);
	fprintf(jf(),"param maxAngularCorrectionDeg %f\n",params->maxAngularCorrectionDeg);
	
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_ray(laser_sens,i)) {
			ld_set_null_correspondence(laser_sens,i);
			continue; 
		}

		transform(laser_sens->p[i], x_old, p_i_w);
		
		int j1 = -1;
		double best_dist = 0;
		
		int from; int to; int start_cell;
		possible_interval(p_i_w, laser_ref, params->maxAngularCorrectionDeg,
			params->maxLinearCorrection, &from, &to, &start_cell);

//		printf("i=%d p_i_w = %f %f %f [from %d to %d]\n",
//			i,gvg(p_i_w,0),gvg(p_i_w,1),gvg(p_i_w,2),from,to);
		
//		from = 0; to = laser_ref->nrays;
		
	//	journal_point("p_i", laser_sens->p[i]);
	//	journal_point("p_i_w", p_i_w);
		int j;
//		if(i==1) fprintf(jf(), "START j1 = %d best_dist = %f\n", j1,best_dist);
		for(j=from;j<=to;j++) {
			if(!ld_valid_ray(laser_ref,j)) continue;
			
			double dist = distance(p_i_w, laser_ref->p[j]);
//			journal_point("p_j", laser_ref->p[j]);
//			if(i==1) 
//						fprintf(jf(),"   i=%d j=%d p_j=(%f, %f) p_i_w =(%f,%f) dist = %f best_j=%d\n", i,j,
//				gvg(laser_ref->p[j],0), gvg(laser_ref->p[j],1), 
//				gvg(p_i_w,0), gvg(p_i_w,1),dist,j1);

			if(dist>params->maxCorrespondenceDist) continue;
			
			
			if((j1==-1) || (dist<best_dist)) {
				j1 =j; 
				best_dist = dist;
//					if(i==1) 	fprintf(jf(), "now j1 = %d best_dist = %f", j1,best_dist);
			} 
		}
		
		if(j1==-1) {// no match
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		// Do not match with extrema
		if(j1==0 || (j1 == (laser_ref->nrays-1))) {// no match
			ld_set_null_correspondence(laser_sens, i);
			continue;
		}
		

/*		double dist = distance(p_i_w, laser_ref->p[j1]);
			fprintf(jf(),"%d -> %d dist = %f p_j=(%f, %f) p_i =(%f,%f)\n ", i,j1,dist,
			gvg(laser_ref->p[j1],0), gvg(laser_ref->p[j1],1),
			gvg(laser_sens->p[i],0), gvg(laser_sens->p[i],1));
*/		
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
		
		ld_set_correspondence(laser_sens, i, j1, j2);
	//	fprintf(jf(), "find_correspondences i=%d from=%d to=%d j1=%d j2=%d\n",i,from,to,j1,j2);
	}
	
}
