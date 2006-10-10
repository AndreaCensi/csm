#include <math.h>
#include "math_utils.h"
#include "laser_data.h"
#include "sm.h"
#include "journal.h"

void quicksort(double *array, int begin, int end);

// expects cartesian valid
void visibilityTest(LDP laser_ref, const gsl_vector*u) {

	double theta_from_u[laser_ref->nrays];
	
	int j;
	for(j=0;j<laser_ref->nrays;j++) {
		if(!ld_valid_ray(laser_ref,j)) continue;
		theta_from_u[j] = 
			atan2(gvg(u,1)-gvg(laser_ref->p[j],1),gvg(u,0)-gvg(laser_ref->p[j],0));
	}
	
	printf("visibility: Found outliers: ");
	int invalid = 0;
	for(j=1;j<laser_ref->nrays;j++) {
		if(!ld_valid_ray(laser_ref,j)||!ld_valid_ray(laser_ref,j-1)) continue;
		if(theta_from_u[j]<theta_from_u[j-1]) {
			laser_ref->valid[j] = 0;
			invalid ++;
			printf("%d ",j);
		}
	}
	printf("\n");
}

void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old, 
	double*total_error) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
	
	int k = 0; 
	double dist[laser_sens->nrays];
	double dist2[laser_sens->nrays];
		
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->corr[i].valid) continue;
		transform(laser_sens->p[i], x_old, p_i_w);
		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;
//		dist[i] = distance(p_i_w, laser_ref->p[j1]);
		dist[i] = dist_to_segment(laser_ref->p[j1],laser_ref->p[j2],p_i_w);
		dist2[k] = dist[i];
		k++;	
	}
	gsl_vector_free(p_i_w);
	
	quicksort(dist2, 0, k-1);
/*	printf("Ordered: ");
	for(i=0;i<k;i++)
		printf("%f ", dist2[i]);
	printf("\n");*/
	
	double error_limit1 = dist2[(int)floor(k*(params->outliers_maxPerc))];
	double error_limit2 = 2*dist2[(int)floor(k*0.7)];
	//double error_limit = 2*dist2[(int)floor(k*0.8)];
	
	double error_limit = GSL_MIN(error_limit1,error_limit2);
	printf("icp_outliers: maxPerc %f error_limit: fix %f adaptive %f \n",
		params->outliers_maxPerc,error_limit1,error_limit2);
	
	*total_error = 0;
	int nvalid = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->corr[i].valid) continue;
		if(dist[i] > error_limit) {
		//	printf("killing %d %d (%f>%f)\n",i,laser_sens->corr[i].j1,dist[i],error_limit);
			laser_sens->corr[i].valid = 0;
			laser_sens->corr[i].j1 = -1;
			laser_sens->corr[i].j2 = -1;
		}
		else {
			nvalid++;
			*total_error += dist[i];
		}
	}
	
	printf("\ticp_outliers: valid %d/%d (limit: %f) mean error = %f \n",nvalid,k,error_limit,
		*total_error/nvalid);	
}


inline void swap_double(double*a,double*b) {
	double t = *a; *a = *b; *b=t;
}

// Code taken from Wikipedia
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




