#include <math.h>
#include "math_utils.h"
#include "laser_data.h"
#include "icp.h"
#include "journal.h"

void quicksort(double *array, int begin, int end);

void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old, double perc) {
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
		dist[i] = distance(p_i_w, laser_ref->p[j1]);
		dist2[k] = dist[i];
		k++;	
	}
	gsl_vector_free(p_i_w);
	
	quicksort(dist2, 0, k-1);
/*	printf("Ordered: ");
	for(i=0;i<k;i++)
		printf("%f ", dist[i]);
	printf("\n");*/
	
	//double error_limit = 2*dist2[(int)floor(k*0.8)];
	
	double error_limit = dist2[(int)floor(k*params->outliers_maxPerc)];
	
	printf("icp_outliers: error_limit: %f \n",error_limit);
	
	int nvalid = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->corr[i].valid) continue;
		if(dist[i] > error_limit) {
		//	printf("killing %d %d (%f>%f)\n",i,laser_sens->corr[i].j1,dist[i],error_limit);
			laser_sens->corr[i].valid = 0;
			laser_sens->corr[i].j1 = -1;
			laser_sens->corr[i].j2 = -1;
		}
		else
			nvalid++;
	}
	
	printf("icp_outliers: valid %d/%d (limit: %f)\n",nvalid,k,error_limit);	
}


void swap_double(double*a,double*b) {
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




