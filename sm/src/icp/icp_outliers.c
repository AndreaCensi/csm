#include <math.h>
#include "../json_journal.h"
#include "../math_utils.h"
#include "../laser_data.h"
#include "../sm.h"
#include "../journal.h"
#include "../logging.h"

void quicksort(double *array, int begin, int end);

/** expects cartesian valid */
void visibilityTest(LDP laser_ref, const gsl_vector*u) {

	double theta_from_u[laser_ref->nrays];
	
	int j;
	for(j=0;j<laser_ref->nrays;j++) {
		if(!ld_valid_ray(laser_ref,j)) continue;
		theta_from_u[j] = 
			atan2(gvg(u,1)-gvg(laser_ref->p[j],1),gvg(u,0)-gvg(laser_ref->p[j],0));
	}
	
	sm_debug("visibility: Found outliers: ");
	int invalid = 0;
	for(j=1;j<laser_ref->nrays;j++) {
		if(!ld_valid_ray(laser_ref,j)||!ld_valid_ray(laser_ref,j-1)) continue;
		if(theta_from_u[j]<theta_from_u[j-1]) {
			laser_ref->valid[j] = 0;
			invalid ++;
			sm_debug("%d ",j);
		}
	}
	sm_debug("\n");
}


/** 
	If multiple points in laser_sens match to the same point in laser_ref, 
	only the nearest one wins.

	Uses: laser_sens->corr, laser_sens->p
	Modifies: laser_sens->corr
 */
void kill_outliers_double(struct sm_params*params, const gsl_vector*x_old) {
	double threshold = 3;

	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);

	double dist_i[laser_sens->nrays];
	double dist_j[laser_ref->nrays];
	int j; for(j=0;j<laser_ref->nrays;j++) 
		dist_j[j]= 1000;
	
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens, i)) continue;
		transform(laser_sens->p[i], x_old, p_i_w);
		int j1 = laser_sens->corr[i].j1;
		dist_i[i] = distance(p_i_w, laser_ref->p[j1]);
		dist_j[j1] = GSL_MIN(dist_j[j1], dist_i[i]);
	}
	
	int nkilled = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens, i)) continue;
		int j1 = laser_sens->corr[i].j1;
		if(dist_i[i] > threshold*dist_j[j1]) {
			laser_sens->corr[i].valid=0;
			nkilled ++;
		}
	}
	sm_debug("kill_outliers_double: killed %d correspondences\n",nkilled);
	gsl_vector_free(p_i_w);
}
	
/** 
	Trims the corrispondences using an adaptive algorithm 

	Assumes cartesian coordinates computed.
	 
	So, to disable this:
		outliers_maxPerc = 1
		outliers_adaptive_order = 1 

*/
void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old, 
	double*total_error) {
		
	if(JJ) jj_context_enter("kill_outliers_trim");
		
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
	
	/* dist2, indexed by k, contains the error for the k-th correspondence */
	int k = 0; 
	double dist2[laser_sens->nrays];
		
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	int i;
	double dist[laser_sens->nrays];
	/* for each point in laser_sens */
	for(i=0;i<laser_sens->nrays;i++) {
		/* which has a valid correspondence */
		if(!ld_valid_corr(laser_sens, i)) { dist[i]=NAN; continue; }
		/* transform its cartesian position, according to current estimate
		   x_old, to obtain: p_i_w, that is the point in the reference 
		   frame of laser_ref */
		transform(laser_sens->p[i], x_old, p_i_w);
		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;
		/* Compute the distance to the corresponding segment */
		dist[i] = dist_to_segment(laser_ref->p[j1],laser_ref->p[j2],p_i_w);
			/* dist[i] = distance(p_i_w, laser_ref->p[j1]); */
		dist2[k] = dist[i];
		k++;	
	}
	gsl_vector_free(p_i_w);
	
	if(JJ) jj_add_int("num_valid_before", k);
	if(JJ) jj_add_double_array("dist_points", dist2, laser_sens->nrays);
	if(JJ) jj_add_double_array("dist_corr_unsorted", dist2, k);
	
	/* The dists for the correspondence are sorted
	   in ascending order */
	quicksort(dist2, 0, k-1);

	if(JJ) jj_add_double_array("dist_corr_sorted", dist2, k);
	
	/* two errors limits are defined: */
		/* In any case, we don't want more than outliers_maxPerc% */
		int order = (int)floor(k*(params->outliers_maxPerc));
			order = GSL_MAX(0, GSL_MIN(order, k-1));
		double error_limit1 = dist2[order];
	
		/* Then we take a order statics (o*K) */
		/* And we say that the error must be less than alpha*dist(o*K) */
		int order2 = (int)floor(k*params->outliers_adaptive_order);
			order2 = GSL_MAX(0, GSL_MIN(order2, k-1));
		double error_limit2 = params->outliers_adaptive_mult*dist2[order2];
	
	double error_limit = GSL_MIN(error_limit1,error_limit2);
	
	if(JJ) jj_add_double("error_limit_max_perc", error_limit1);
	if(JJ) jj_add_double("error_limit_adaptive", error_limit2);
	if(JJ) jj_add_double("error_limit", error_limit);
	
	sm_debug("icp_outliers: maxPerc %f error_limit: fix %f adaptive %f \n",
		params->outliers_maxPerc,error_limit1,error_limit2);

	*total_error = 0;
	int nvalid = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens, i)) continue;
		if(dist[i] > error_limit) {
			laser_sens->corr[i].valid = 0;
			laser_sens->corr[i].j1 = -1;
			laser_sens->corr[i].j2 = -1;
		} else {
			nvalid++;
			*total_error += dist[i];
		}
	}
	
	sm_debug("\ticp_outliers: valid %d/%d (limit: %f) mean error = %f \n",nvalid,k,error_limit,
		*total_error/nvalid);	

	if(JJ) jj_add_int("num_valid_after", nvalid);
	if(JJ) jj_add_double("total_error", *total_error);
	if(JJ) jj_add_double("mean_error", *total_error / nvalid);
		
	if(JJ) jj_context_exit();
}


void swap_double(double*a,double*b) {
	double t = *a; *a = *b; *b=t;
}

/** Code taken from Wikipedia */
void quicksort(double *array, int begin, int end) {
	if (end > begin) {
	   double pivot = array[begin];
	   int l = begin + 1;
	   int r = end+1;
	   while(l < r) {
	      if (array[l] < pivot) {
	         l++;
	      } else {
	         r--;
	         swap_double(array+l, array+r); 
	      }
	   }
	   l--;
	   swap_double(array+begin, array+l);
	  quicksort(array, begin, l);
	  quicksort(array, r, end);
	}
}




