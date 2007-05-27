#include <math.h>
#include "laser_data.h"

/*
/// A very very simple clustering algorithm.
/// Try threshold = 5*sigma */

void ld_simple_clustering(LDP ld, double threshold) {	
	int cluster = -1;
	double last_reading;
	
	int i;
	for(i=0;i<ld->nrays;i++) {
		/* Skip if not valid */
		if(!ld_valid_ray(ld,i)) {
			ld->cluster[i] = -1;
			continue;
		}
		/* If this is the first valid point, assign cluster #0 */
		if(-1==cluster) 
			cluster = 0;
		else 
		/* Else, start a new cluster if distance is above threshold */
		if( fabs(last_reading-ld->readings[i]) > threshold)
			cluster++;
				
		ld->cluster[i] = cluster;
		last_reading = ld->readings[i];
	}
}

