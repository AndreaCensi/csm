#include <math.h>
#include <gsl/gsl_histogram.h>

#include <gpc.h>

#include "math_utils.h"
#include "laser_data.h"
#include "icp.h"
#include "journal.h"


void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new,
	double *error);
int termination_criterion(gsl_vector*delta, struct icp_input*params);

void find_correspondences_tricks(struct icp_input*params, gsl_vector* x_old);
void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid);
void icp_loop(struct icp_input*params, const gsl_vector*start, gsl_vector*x_new, double*error, int*iterations);
//void kill_outliers_trim(int K, struct gpc_corr*c, const gsl_vector*x_old, double perc, int*valid);
void kill_outliers_trim(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, double perc);

void icp(struct icp_input*params, struct icp_output*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
		
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);
	
	ld_create_jump_tables(laser_ref);
	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);
	ld_compute_orientation(laser_ref, 3);
	ld_compute_orientation(laser_sens, 3);
		
	gsl_vector * x_new = gsl_vector_alloc(3);
	gsl_vector * x_old = vector_from_array(3, params->odometry);
	
	double error;
	int iterations;
	icp_loop(params, x_old, x_new, &error, &iterations);

	double best_error = error;
	gsl_vector * best_x = gsl_vector_alloc(3);
	gsl_vector_memcpy(best_x, x_new);

	if(params->restart) {
		double dt  = params->restart_dt;
		double dth = params->restart_dtheta;
		
		
		double perturb[2][3] = {
//			{dt,0,0}, {-dt,0,0},
//			{0,dt,0}, {0,-dt,0},
			{0,0,dth}, {0,0,-dth}
		};

		int a; for(a=0;a<2;a++){
			printf("-- Restarting with perturbation #%d\n", a);
			struct icp_input my_params = *params;
			gsl_vector * start = gsl_vector_alloc(3);
				gvs(start, 0, gvg(x_new,0)+perturb[a][0]);
				gvs(start, 1, gvg(x_new,1)+perturb[a][1]);
				gvs(start, 2, gvg(x_new,2)+perturb[a][2]);
			gsl_vector * x_a = gsl_vector_alloc(3);
			double my_error; int my_iterations;
			icp_loop(&my_params, start, x_a, &my_error, &my_iterations);
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
}

unsigned int ld_corr_hash(LDP ld){
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < ld->nrays; i++) {
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

void icp_loop(struct icp_input*params, const gsl_vector*start, gsl_vector*x_new, double*error, int*iterations) {
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
		kill_outliers_trim(laser_ref, laser_sens, x_old, 0.9);
		
		int num_corr = ld_num_valid_correspondences(laser_sens);
		if(num_corr <0.2 * laser_sens->nrays){
			printf("Failed: only %d correspondences.\n",num_corr);
			break;
		}
		journal_correspondences(laser_sens);
		compute_next_estimate(laser_ref, laser_sens, x_old, x_new, error);
		
		pose_diff(x_new, x_old, delta);
		
		journal_pose("x_new", x_new);
		journal_pose("delta", delta);

		double delta_sign = gvg(delta,0)*gvg(delta_old,0)+gvg(delta,1)*gvg(delta_old,1)+gvg(delta,2)*gvg(delta_old,2);
		
		if(delta_sign<0)
			oscillations ++;
		else 
			oscillations = 0;
		
		hashes[iteration] = ld_corr_hash(laser_sens);
		printf("icp_loop: it. %d  hash = %d error = %f, x_new= %f %f %f°  \n", 
			iteration, hashes[iteration], *error, gvg(x_new,0),gvg(x_new,1),rad2deg(gvg(x_new,2)));
			
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

int termination_criterion(gsl_vector*delta, struct icp_input*params){
	double a = sqrt(gvg(delta,0)* gvg(delta,0)+ gvg(delta,1)* gvg(delta,1));
	double b = fabs(gvg(delta,2));
	return (a<params->epsilon_xy) && (b<params->epsilon_theta);
}

void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new,
	double *error) {
	struct gpc_corr c[laser_sens->nrays];

	*error = 0;
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

	//	c[k].C[0][0] += 0.02;
	//	c[k].C[1][1] += 0.02;
		*error += gpc_error(c+k, x_old->data);	
		
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
	
