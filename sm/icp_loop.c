#include <math.h>
#include <gsl/gsl_histogram.h>

#include <gpc.h>

#include "math_utils.h"
#include "laser_data.h"
#include "sm.h"
#include "journal.h"


void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new);
int termination_criterion(gsl_vector*delta, struct sm_params*params);

void find_correspondences_tricks(struct sm_params*params, gsl_vector* x_old);
void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid);
void icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
 	double*total_error, int*nvalid, int*iterations);

void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old,
	double*total_error);

void sm_icp(struct sm_params*params, struct sm_result*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
			
	ld_create_jump_tables(laser_ref);
		
	ld_compute_cartesian(laser_ref);

	ld_compute_cartesian(laser_sens);

	if(params->doAlphaTest) {
		ld_simple_clustering(laser_ref, params->clusteringThreshold);
		ld_compute_orientation(laser_ref, params->orientationNeighbourhood, params->sigma);
		ld_simple_clustering(laser_sens, params->clusteringThreshold);
		ld_compute_orientation(laser_sens, params->orientationNeighbourhood, params->sigma);
	}
	
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);
		
	gsl_vector * x_new = gsl_vector_alloc(3);
	gsl_vector * x_old = vector_from_array(3, params->odometry);
	
	if(params->doVisibilityTest) {
		printf("laser_ref:\n");
		visibilityTest(laser_ref, x_old);

		printf("laser_sens:\n");
		gsl_vector * minus_x_old = gsl_vector_alloc(3);
		ominus(x_old,minus_x_old);
		visibilityTest(laser_sens, minus_x_old);
		gsl_vector_free(minus_x_old);
	}
	
	double error;
	int iterations;
	int nvalid;
	icp_loop(params, x_old, x_new, &error, &nvalid,&iterations);

	double best_error = error;
	gsl_vector * best_x = gsl_vector_alloc(3);
	gsl_vector_memcpy(best_x, x_new);

	if(params->restart && 
		(error/nvalid)>(params->restart_threshold_mean_error) ) {
		printf("Restarting: %f > %f \n",(error/nvalid),(params->restart_threshold_mean_error));
		double dt  = params->restart_dt;
		double dth = params->restart_dtheta;
		printf("icp_loop: dt = %f dtheta= %f deg\n",dt,rad2deg(dth));
		
		double perturb[6][3] = {
			{dt,0,0}, {-dt,0,0},
			{0,dt,0}, {0,-dt,0},
			{0,0,dth}, {0,0,-dth}
		};

		int a; for(a=0;a<6;a++){
			printf("-- Restarting with perturbation #%d\n", a);
			struct sm_params my_params = *params;
			gsl_vector * start = gsl_vector_alloc(3);
				gvs(start, 0, gvg(x_new,0)+perturb[a][0]);
				gvs(start, 1, gvg(x_new,1)+perturb[a][1]);
				gvs(start, 2, gvg(x_new,2)+perturb[a][2]);
			gsl_vector * x_a = gsl_vector_alloc(3);
			double my_error; int my_valid; int my_iterations;
			icp_loop(&my_params, start, x_a, &my_error, &my_valid, &my_iterations);
			iterations+=my_iterations;
		
			if(my_error < best_error) {
				printf("--Perturbation #%d resulted in error %f < %f\n", a,my_error,best_error);
				gsl_vector_memcpy(best_x, x_a);
				best_error = my_error;
			}
			gsl_vector_free(x_a); gsl_vector_free(start);
		}
	}
	
	vector_to_array(best_x, res->x);
	res->error = best_error;
	res->iterations = iterations;
	res->nvalid = nvalid;
}

unsigned int ld_corr_hash(LDP ld){
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < (unsigned)ld->nrays; i++) {
		int str = ld->corr[i].valid ? ld->corr[i].j1 : -1;
		hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (str) ^ (hash >> 3)) :
		                         (~((hash << 11) ^ (str) ^ (hash >> 5)));
	}

	return (hash & 0x7FFFFFFF);
}

int ld_num_valid_correspondences(LDP ld) {
	int i; 
	int num = 0;
	for(i=0;i<ld->nrays;i++) {
		if(ld->corr[i].valid)
			num++;
	}
	return num;
}

void icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
	double*total_error, int*valid, int*iterations) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
	
	gsl_vector * x_old = vector_from_array(3, start->data);
	gsl_vector * delta = gsl_vector_alloc(3);
	gsl_vector * delta_old = gsl_vector_alloc(3);
	gsl_vector_set_all(delta_old,0.0);

	int oscillations = 0;
	unsigned int hashes[params->maxIterations];
	int iteration;

	printf("icp_loop: starting at x_old= %f %f %f°  \n", 
		gvg(x_old,0),gvg(x_old,1),rad2deg(gvg(x_old,2)));
	
	for(iteration=0; iteration<params->maxIterations;iteration++) {
		if(jf()) fprintf(jf(), "iteration %d\n", iteration);
		journal_pose("x_old", x_old);
		
//		find_correspondences(params, x_old);
		find_correspondences_tricks(params, x_old);
		int num_corr = ld_num_valid_correspondences(laser_sens);
		if(num_corr <0.2 * laser_sens->nrays){
			printf("Failed: before trimming, only %d correspondences.\n",num_corr);
			break;
		}

		double error;
		kill_outliers_trim(params, x_old, &error);
		int num_corr_after = ld_num_valid_correspondences(laser_sens);
		
		*total_error = error; 
		*valid = num_corr_after;
		
		if(num_corr_after <0.2 * laser_sens->nrays){
			printf("Failed: after trimming, only %d correspondences.\n",num_corr_after);
			break;
		}
		journal_correspondences(laser_sens);
		compute_next_estimate(laser_ref, laser_sens, x_old, x_new);
		
		pose_diff(x_new, x_old, delta);
		
		journal_pose("x_new", x_new);
		journal_pose("delta", delta);

		double delta_sign = gvg(delta,0)*gvg(delta_old,0)+gvg(delta,1)*gvg(delta_old,1)+gvg(delta,2)*gvg(delta_old,2);
		
		if(delta_sign<0)
			oscillations ++;
		else 
			oscillations = 0;
		
		hashes[iteration] = ld_corr_hash(laser_sens);
		printf("icp_loop: it. %d  hash = %d nvalid=%d mean error = %f, x_new= %f %f %f°  \n", 
			iteration, hashes[iteration], *valid, *total_error/ *valid, 
			gvg(x_new,0),gvg(x_new,1),rad2deg(gvg(x_new,2)));
			
		int detected = 0;
		int a; for(a=0;a<iteration;a++) {
			if(hashes[a]==hashes[iteration]) {
				printf("icpc: oscillation detected (cycle length = %d)\n", iteration-a);
				detected = 1;
			}
		}
		if(detected) break;
/*
		if( (iteration>5 && oscillations>=3) || (iteration>10 && oscillations>=2)){
			printf("icpc: oscillation detected \n");
//			break;
		}*/
		if(termination_criterion(delta, params)) {
			break;
		}
		
		gsl_vector_memcpy(x_old, x_new);
		gsl_vector_memcpy(delta_old, delta);
	}
	
	// TODO: covariance
	*iterations = iteration+1;
	
	gsl_vector_free(x_old);
	gsl_vector_free(delta);
	gsl_vector_free(delta_old);
}

int termination_criterion(gsl_vector*delta, struct sm_params*params){
	double a = sqrt(gvg(delta,0)* gvg(delta,0)+ gvg(delta,1)* gvg(delta,1));
	double b = fabs(gvg(delta,2));
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

void compute_next_estimate(LDP laser_ref, LDP laser_sens, 
	const gsl_vector*x_old, gsl_vector*x_new) {
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

	//	c[k].C[0][0] = 1;
	//	c[k].C[1][0] = 
	//	c[k].C[0][1] = 0;
	//	c[k].C[1][1] = 1;

	//	c[k].C[0][0] += 0.02;
	//	c[k].C[1][1] += 0.02;
		k++;
	}

	int valid[k];
	int kk; for(kk=0;kk<k;kk++) valid[kk]=1;
	
	//kill_outliers(k, c, x_old, valid);
///	kill_outliers_trim(k,c,x_old,0.9,valid);
	
	journal_write_array_i("valid", k, valid);
	
	double x[3];
	gpc_solve_valid(k, c, valid, x);
	
	gvs(x_new,0,x[0]);
	gvs(x_new,1,x[1]);
	gvs(x_new,2,x[2]);
}
	


