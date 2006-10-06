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


void kill_outliers_trim(int K, struct gpc_corr*c, const gsl_vector*x_old, double perc, int*valid) {
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

	double error_limit; double dummy;
	gsl_histogram_get_range(hist, max_bin, &dummy, &error_limit);
	
	int nvalid = 0;
	for(k=0;k<K;k++) {
		valid[k] = dist[k] > error_limit? 0 : 1;
		nvalid += valid[k];
	}
	
	
	printf("icp_outliers: %d/%d\n",nvalid,K);
	
	gsl_histogram_free(hist);
}
