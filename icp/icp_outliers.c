#include <math.h>
#include <gsl/gsl_histogram.h>

#include <gpc.h>

#include "math_utils.h"
#include "laser_data.h"
#include "icp.h"
#include "journal.h"

/*	#define NBINS 200
	#define MAX_RANGE 0.5
	#define PERC 0.9
	#define SAFE_LEVEL 1*/
	#define NBINS 200
	#define MAX_RANGE 0.5
	#define PERC 0.9
	#define SAFE_LEVEL 1

void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid) {
	int k;
	double dist[K];
	
	gsl_histogram * hist = gsl_histogram_alloc(NBINS);
	gsl_histogram_set_ranges_uniform(hist, 0.0, MAX_RANGE);
	
	for(k=0;k<K;k++) {
		dist[k] = gpc_error(c+k, x_old->data);	
		if(dist[k]>MAX_RANGE) continue;
		gsl_histogram_increment(hist, dist[k]);
	}
	
	double integral=0;
	unsigned int b;
	for(b=0;b<NBINS;b++){
		integral += gsl_histogram_get(hist,b);
		if(integral>PERC*K)
			break;
	}
	size_t max_bin = (size_t) b;
	double mean=0; int mean_count=0;
	// now compute mean for elements in bin < b
	for(k=0;k<K;k++) {
		if(dist[k]>MAX_RANGE) continue;
		size_t bin; gsl_histogram_find(hist, dist[k], &bin);
		if(bin <= max_bin) {
			mean += dist[k];
			mean_count++;
		}
	}
	mean /= mean_count;
	
	double error_limit = mean*SAFE_LEVEL; 
	int nvalid = 0;
	for(k=0;k<K;k++) {
		valid[k] = dist[k] > error_limit? 0 : 1;
		nvalid += valid[k];
	}
	
	if(jf()) fprintf(jf(),"bins=%d max_range=%f perc=%f ",NBINS,MAX_RANGE,PERC);
	if(jf()) fprintf(jf(),
		" max_bin=%d mean=%f error_limit=%f nvalid=%d/%d \n",
		max_bin,mean,error_limit,nvalid,K);

/*	for(k=0;k<K;k++) {
		printf("%d %f\n",valid[k],dist[k]);
	}*/
	
	printf("icp_outliers: %d/%d\n",nvalid,K);
	
	gsl_histogram_free(hist);
}

void swap_double(double*a,double*b) {
	double t = *a; *a = *b; *b=t;
}

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

void kill_outliers_trim(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, double perc) {
	int k = 0; 
	double dist[laser_sens->nrays];
	double dist2[laser_sens->nrays];
		
//	gsl_histogram * hist = gsl_histogram_alloc(NBINS);
//	gsl_histogram_set_ranges_uniform(hist, 0.0, MAX_RANGE);
	gsl_vector * p_i_w = gsl_vector_alloc(3);
	
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->corr[i].valid) continue;
		transform(laser_sens->p[i], x_old, p_i_w);
		int j1 = laser_sens->corr[i].j1;
		dist[i] = distance(p_i_w, laser_ref->p[j1]);
		dist2[k] = dist[i];
//		if(dist[k]>MAX_RANGE) continue;
//		gsl_histogram_increment(hist, dist[k]);
		k++;
	}

	quicksort(dist2, 0, k-1);
	printf("Ordered: ");
	for(i=0;i<k;i++)
		printf("%f ", dist[i]);
	printf("\n");
	
	double error_limit = 2*dist2[(int)floor(k*0.8)];
	double max_limit = dist2[(int)floor(k*0.9)];
	
	printf("error_limit: %f  max_limit %f\n",error_limit,max_limit);
	error_limit= GSL_MIN(error_limit, max_limit);
	
	int nvalid = 0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->corr[i].valid) continue;
		if(dist[i] > error_limit) {
			printf("killing %d %d (%f>%f)\n",i,laser_sens->corr[i].j1,dist[i],error_limit);
			laser_sens->corr[i].valid = 0;
			laser_sens->corr[i].j1 = -1;
			laser_sens->corr[i].j2 = -1;
		}
		else
			nvalid++;
	}
	
	printf("icp_outliers: valid %d/%d (limit: %f)\n",nvalid,k,error_limit);
	
	gsl_vector_free(p_i_w);
}
