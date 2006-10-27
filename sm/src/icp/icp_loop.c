#include <math.h>
#include <gsl/gsl_matrix.h>

#include <gpc.h>
#include <egsl_macros.h>

#include "../math_utils.h"
#include "../laser_data.h"
#include "../sm.h"
#include "../journal.h"

//#define EXPERIMENT_COVARIANCE


void visibilityTest(LDP ld, const gsl_vector*x_old);

void compute_next_estimate(LDP laser_ref, LDP laser_sens, const gsl_vector*x_old, gsl_vector*x_new);
int termination_criterion(gsl_vector*delta, struct sm_params*params);

void find_correspondences_tricks(struct sm_params*params, gsl_vector* x_old);
void kill_outliers(int K, struct gpc_corr*c, const gsl_vector*x_old, int*valid);
void icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
 	double*total_error, int*nvalid, int*iterations);

void kill_outliers_trim(struct sm_params*params, const gsl_vector*x_old,
	double*total_error);
void kill_outliers_double(struct sm_params*params, const gsl_vector*x_old);
	
void compute_covariance_exact(
	LDP laser_ref, LDP laser_sens, const gsl_vector*x,
		val *cov0_x, val *dx_dy1, val *dx_dy2);

void sm_icp(struct sm_params*params, struct sm_result*res) {
	egsl_push();
	
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
			
	if(params->useCorrTricks)
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
	
	
	if(params->doComputeCovariance)  {

		val cov0_x, dx_dy1, dx_dy2;
		compute_covariance_exact(
			laser_ref, laser_sens, best_x,
			&cov0_x, &dx_dy1, &dx_dy2);
		
//		val cov_x = sc(params->sigma*params->sigma, cov0_x);
		
		egsl_print("cov0_x", cov0_x);
		egsl_print_spectrum("cov0_x", cov0_x);
		
		val fim = ld_fisher0(laser_ref);
		val ifim = inv(fim);
		egsl_print("fim", fim);
		egsl_print_spectrum("ifim", ifim);
	}
	
	
	
	res->error = best_error;
	res->iterations = iterations;
	res->nvalid = nvalid;
	
	egsl_pop();
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

void icp_loop(struct sm_params*params, const gsl_vector*start, gsl_vector*x_new, 
	double*total_error, int*valid, int*iterations) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
	
	gsl_vector * x_old = vector_from_array(3, start->data);
	gsl_vector * delta = gsl_vector_alloc(3);
	gsl_vector * delta_old = gsl_vector_alloc(3);
	gsl_vector_set_all(delta_old,0.0);

	unsigned int hashes[params->maxIterations];
	int iteration;

	printf("icp_loop: starting at x_old= %s  \n",
		gsl_friendly_pose(x_old));
	
	#ifdef EXPERIMENT_COVARIANCE
		printf("icp_cov next\n");
	#endif
	
	for(iteration=0; iteration<params->maxIterations;iteration++) {
		egsl_push();
		
		if(jf()) fprintf(jf(), "iteration %d\n", iteration);
		journal_pose("x_old", x_old);
		
		if(params->useCorrTricks)
			find_correspondences_tricks(params, x_old);
		else
			find_correspondences(params, x_old);

		int num_corr = ld_num_valid_correspondences(laser_sens);
		if(num_corr <0.2 * laser_sens->nrays){
			egsl_pop();
			printf("Failed: before trimming, only %d correspondences.\n",num_corr);
			break;
		}

//		kill_outliers_double(params, x_old);
		int num_corr2 = ld_num_valid_correspondences(laser_sens);

		double error=0;
		kill_outliers_trim(params, x_old, &error);
		int num_corr_after = ld_num_valid_correspondences(laser_sens);
		
		*total_error = error; 
		*valid = num_corr_after;
		
		if(num_corr_after <0.2 * laser_sens->nrays){
			printf("Failed: after trimming, only %d correspondences.\n",num_corr_after);
			egsl_pop();
			break;
		}
		journal_correspondences(laser_sens);
		compute_next_estimate(laser_ref, laser_sens, x_old, x_new);
		
		pose_diff(x_new, x_old, delta);
		
		
		printf("killing %d -> %d -> %d \n", num_corr, num_corr2, num_corr_after);
		journal_pose("x_new", x_new);
		journal_pose("delta", delta);


		/** Checks for oscillations */
		hashes[iteration] = ld_corr_hash(laser_sens);
		printf("icp_loop: it. %d  hash=%d nvalid=%d mean error = %f, x_new= %s\n", 
			iteration, hashes[iteration], *valid, *total_error/ *valid, 
			gsl_friendly_pose(x_new));

#ifdef EXPERIMENT_COVARIANCE
				val cov0_x, dx_dy1, dx_dy2;
				compute_covariance_exact(
					laser_ref, laser_sens, x_new,
					&cov0_x, &dx_dy1, &dx_dy2);
		//		egsl_print_spectrum("cov0_x", cov0_x);

				val cov_x = sc(square(params->sigma), cov0_x);
		printf("icp_cov x_new %f %f %f \n",
			gvg(x_new,0),gvg(x_new,1),gvg(x_new,2));
			
		printf("icp_cov cov0 ");
			size_t i,j;
			for(i=0;i<3;i++) {
				printf("\t");
				for(j=0;j<3;j++) {
					printf("%f ", egsl_atm(cov0_x, i, j) );
				} 
			}

		printf("\n");
		
#endif
		egsl_pop();
						
		int detected = 0;
		int a; for(a=0;a<iteration;a++) {
			if(hashes[a]==hashes[iteration]) {
				printf("icpc: oscillation detected (cycle length = %d)\n", iteration-a);
				detected = 1;
			}
		}
		if(detected) break;

		if(termination_criterion(delta, params)) 
			break;
		
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
		
	journal_write_array_i("valid", k, valid);
	
	double x[3];
	gpc_solve_valid(k, c, valid, x);
	
	gvs(x_new,0,x[0]);
	gvs(x_new,1,x[1]);
	gvs(x_new,2,x[2]);
}
	


