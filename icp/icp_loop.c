#include <math.h>
#include <gsl/gsl_histogram.h>

#include <gpc.h>

#include "math_utils.h"
#include "laser_data.h"
#include "icp.h"
#include "journal.h"

void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new);
int termination_criterion(gsl_vector*delta, struct icp_input*params);

void find_correspondences_tricks(struct icp_input*params, gsl_vector* x_old);

void icp(struct icp_input*params, struct icp_output*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
		
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);
	
	ld_create_jump_tables(laser_ref);
	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);
	
	int iteration;
	
	gsl_vector * x_old = vector_from_array(3, params->odometry);
	gsl_vector * x_new = gsl_vector_alloc(3);
	gsl_vector * delta = gsl_vector_alloc(3);
		
	for(iteration=0; iteration<params->maxIterations;iteration++) {
		fprintf(jf(), "iteration %d\n", iteration);
		journal_pose("x_old", x_old);
		
//		find_correspondences(params, x_old);
		find_correspondences_tricks(params, x_old);
		compute_next_estimate(laser_ref, laser_sens, x_old, x_new);
		journal_correspondences(laser_sens);
		
		pose_diff(x_new, x_old, delta);
		
		journal_pose("x_new", x_new);
		journal_pose("delta", delta);

		printf("icpc: it. %d x= %f %f %f\n", 
			iteration,gvg(x_new,0),gvg(x_new,1),gvg(x_new,2));

		if(termination_criterion(delta, params)) {
			break;
		}
		
		gsl_vector_memcpy(x_old, x_new);
	}
	
	gsl_vector_free(x_old);
	gsl_vector_free(x_new);
	gsl_vector_free(delta);
}

int termination_criterion(gsl_vector*delta, struct icp_input*params){
	double a = sqrt(gvg(delta,0)* gvg(delta,0)+ gvg(delta,1)* gvg(delta,1));
	double b = fabs(gvg(delta,2));
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid) {
	int k;
	double dist[K];
	
	#define NBINS 200
	#define MAX_RANGE 2.0
	#define PERC 0.6
	#define SAFE_LEVEL 5.0
	gsl_histogram * hist = gsl_histogram_alloc(NBINS);
	gsl_histogram_set_ranges_uniform(hist, 0.0, MAX_RANGE);
	
	for(k=0;k<K;k++) {
		dist[k] = gpc_error(c+k, x_old->data);	
		if(dist[k]>MAX_RANGE) continue;
		gsl_histogram_increment(hist, dist[k]);
	}
	int half=0;
	double integral=0;
	unsigned int b;
	for(b=0;b<NBINS;b++){
		integral += gsl_histogram_get(hist,b);
		if(integral>PERC*K)
			break;
	}
	int max_bin = b;
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
	
	printf("bins=%d max_range=%f perc=%f ",NBINS,MAX_RANGE,PERC);
	printf(" max_bin=%d mean=%f error_limit=%f nvalid=%d/%d \n",max_bin,mean,error_limit,nvalid,K);

/*	for(k=0;k<K;k++) {
		printf("%d %f\n",valid[k],dist[k]);
	}*/
	
	gsl_histogram_free(hist);
}

void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new) {
	struct gpc_corr c[laser_sens->nrays];

	int i; int k=0;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i))
			continue;
		
		c[k].p[0] = gvg(laser_sens->p[i],0);
		c[k].p[1] = gvg(laser_sens->p[i],1);

		int j1 = laser_sens->corr[i].j1;
		c[k].q[0] = gvg(laser_ref->p[j1],0);
		c[k].q[1] = gvg(laser_ref->p[j1],1);
		
		int j2 = laser_sens->corr[i].j2;
		double alpha = M_PI/2 + atan2( 
			gvg(laser_ref->p[j1],1)-gvg(laser_ref->p[j2],1),
			gvg(laser_ref->p[j1],0)-gvg(laser_ref->p[j2],0));
		
		c[k].C[0][0] = cos(alpha)*cos(alpha);
		c[k].C[1][0] = 
		c[k].C[0][1] = cos(alpha)*sin(alpha);
		c[k].C[1][1] = sin(alpha)*sin(alpha);
		
		k++;
	}

	int valid[k];
	int kk; for(kk=0;kk<k;kk++) valid[kk]=1;
	
	//kill_outliers(k, c, x_old, valid);
	
	double x[3];
	gpc_solve_valid(k, c, valid, x);
	
	gvs(x_new,0,x[0]);
	gvs(x_new,1,x[1]);
	gvs(x_new,2,x[2]);
}

