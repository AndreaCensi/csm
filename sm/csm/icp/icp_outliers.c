#include <math.h>

#include "icp.h"

void quicksort(double *array, int begin, int end);
double hoare_selection(double *data, int start, int end, int k);

/** expects cartesian valid */
void visibilityTest(LDP laser_ref, const gsl_vector*u) {

	double theta_from_u[laser_ref->nrays];
	
	int j;
	for(j=0;j<laser_ref->nrays;j++) {
		if(!ld_valid_ray(laser_ref,j)) continue;
		theta_from_u[j] = 
			atan2(gvg(u,1)-laser_ref->points[j].p[1],
			      gvg(u,0)-laser_ref->points[j].p[0]);
	}
	
	sm_debug("\tvisibility: Found outliers: ");
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
void kill_outliers_double(struct sm_params*params) {
	double threshold = 3; /* TODO: add as configurable */

	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;

	double dist2_i[laser_sens->nrays];
	double dist2_j[laser_ref->nrays];
	int j; for(j=0;j<laser_ref->nrays;j++) 
		dist2_j[j]= 1000000;
	
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens, i)) continue;
		int j1 = laser_sens->corr[i].j1;
		dist2_i[i] = laser_sens->corr[i].dist2_j1;
		dist2_j[j1] = GSL_MIN(dist2_j[j1], dist2_i[i]);
	}
	
	int nkilled = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens, i)) continue;
		int j1 = laser_sens->corr[i].j1;
		if(dist2_i[i] > (threshold*threshold)*dist2_j[j1]) {
			laser_sens->corr[i].valid=0;
			nkilled ++;
		}
	}
	sm_debug("\tkill_outliers_double: killed %d correspondences\n",nkilled);
}
	
/** 
	Trims the corrispondences using an adaptive algorithm 

	Assumes cartesian coordinates computed. (points and points_w)
	 
	So, to disable this:
		outliers_maxPerc = 1
		outliers_adaptive_order = 1 

*/
void kill_outliers_trim(struct sm_params*params,  double*total_error) {
		
	if(JJ) jj_context_enter("kill_outliers_trim");
		
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	
	/* dist2, indexed by k, contains the error for the k-th correspondence */
	int k = 0; 
	double dist2[laser_sens->nrays];
		
	int i;
	double dist[laser_sens->nrays];
	/* for each point in laser_sens */
	for(i=0;i<laser_sens->nrays;i++) {
		/* which has a valid correspondence */
		if(!ld_valid_corr(laser_sens, i)) { dist[i]=NAN; continue; }
		double *p_i_w = laser_sens->points_w[i].p;
		
		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;
		/* Compute the distance to the corresponding segment */
		dist[i]=  dist_to_segment_d(
			laser_ref->points[j1].p, laser_ref->points[j2].p, p_i_w);
		dist2[k] = dist[i];
		k++;	
	}
	
	
	if(JJ) jj_add_int("num_valid_before", k);
	if(JJ) jj_add_double_array("dist_points", dist2, laser_sens->nrays);
	if(JJ) jj_add_double_array("dist_corr_unsorted", dist2, k);

#if 0	
	double dist2_copy[k]; for(i=0;i<k;i++) dist2_copy[i] = dist2[i];
#endif 

	/* two errors limits are defined: */
		/* In any case, we don't want more than outliers_maxPerc% */
		int order = (int)floor(k*(params->outliers_maxPerc));
			order = GSL_MAX(0, GSL_MIN(order, k-1));

	/* The dists for the correspondence are sorted
	   in ascending order */
		quicksort(dist2, 0, k-1);
		double error_limit1 = dist2[order];
		if(JJ) jj_add_double_array("dist_corr_sorted", dist2, k);
	
		/* Then we take a order statics (o*K) */
		/* And we say that the error must be less than alpha*dist(o*K) */
		int order2 = (int)floor(k*params->outliers_adaptive_order);
			order2 = GSL_MAX(0, GSL_MIN(order2, k-1));
		double error_limit2 = params->outliers_adaptive_mult*dist2[order2];
	
	double error_limit = GSL_MIN(error_limit1, error_limit2);
	
#if 0
	double error_limit1_ho = hoare_selection(dist2_copy, 0, k-1, order);
	double error_limit2_ho = error_limit2;
	if((error_limit1_ho != error_limit1) || (error_limit2_ho != error_limit2)) {
		printf("%f == %f    %f  == %f\n",
			error_limit1_ho, error_limit1, error_limit2_ho, error_limit2);
	}
#endif

	if(JJ) jj_add_double_array("dist_corr_sorted", dist2, k);
	if(JJ) jj_add_double("error_limit_max_perc", error_limit1);
	if(JJ) jj_add_double("error_limit_adaptive", error_limit2);
	if(JJ) jj_add_double("error_limit", error_limit);
	
	sm_debug("\ticp_outliers: maxPerc %f error_limit: fix %f adaptive %f \n",
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
		if(l>begin)
		quicksort(array, begin, l);
		if(end>r)
		quicksort(array, r, end);
	}
}

#if 0
double hoare_selection(double *data, int start, int end, int k)
{
  int pivotIndex, i, j;
  float pivotValue, tmp;

  while(start < end) {
	  //select a random pivot
	pivotIndex = start + (end-start)/2;
	 pivotValue = data[pivotIndex];
				 //sort the array into two sub arrays around the pivot value
	 i = start;
	 j = end;
	 while(i <= j) {
		while(data[i] < pivotValue)
		  i++;
		while(data[j] > pivotValue)
		  j--;
		if(i<=j) {
		  tmp = data[i];
		  data[i] = data[j];
		  data[j] = tmp;
		  i++;
		  j--;
		}
	 }
			 //continue search in left sub array
	 if(j < k)
		start = i;
	 if(k < i) //continue search in right sub array
		end = j;
  }
  return(data[k]);
}

#endif
