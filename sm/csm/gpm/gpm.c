#include <gsl/gsl_histogram.h>
#include <gsl/gsl_matrix.h>


#include "../csm_all.h"
#include "gpm.h"

#include <egsl/egsl_macros.h>

	
void sm_gpm(struct sm_params*params, struct sm_result*res) {
	res->valid = 0;
	/* Check for well-formedness of the input data */
	if(!ld_valid_fields(params->laser_ref) || 
	   !ld_valid_fields(params->laser_sens)) {
		return;
	}
	
	LDP laser_ref  = params->laser_ref;
	LDP laser_sens = params->laser_sens;
	
	/* We need to compute cartesian points */
	ld_compute_cartesian(laser_ref);
	/* ... and orientation */
	ld_simple_clustering(laser_ref, params->clustering_threshold);
	ld_compute_orientation(laser_ref, params->orientation_neighbourhood, params->sigma);
	/* ... for both scans. */
	ld_compute_cartesian(laser_sens);
	ld_simple_clustering(laser_sens, params->clustering_threshold);
	ld_compute_orientation(laser_sens, params->orientation_neighbourhood, params->sigma);

	/* Create an histogram whose bin is large `theta_bin_size` */
	double theta_bin_size = deg2rad(params->gpm_theta_bin_size_deg);
	double hist_min = -M_PI-theta_bin_size; /* be robust */
	double hist_max = +M_PI+theta_bin_size;
	size_t nbins = (size_t) ceil(  (hist_max-hist_min) / theta_bin_size);
	gsl_histogram*hist = gsl_histogram_alloc(nbins);
	gsl_histogram_set_ranges_uniform(hist, hist_min, hist_max);
	
	/* Fill the histogram with samples */
	double u[3]; copy_d(params->first_guess, 3, u);
	sm_debug("gpm 1/2: old u = : %s \n", friendly_pose(u) );
	
	int interval = params->gpm_interval;
	
	int num_correspondences_theta=-1;
	
	
	ght_find_theta_range(laser_ref, laser_sens,
		u, params->max_linear_correction,
		params->max_angular_correction_deg, interval, hist, &num_correspondences_theta);
		
	if(num_correspondences_theta < laser_ref->nrays) {
		sm_error("sm_gpm(): I found only %d correspondences in the first pass of GPM. I consider it a failure.\n",
			num_correspondences_theta);
		return;
	}
	
	/* Find the bin with most samples */
	size_t max_bin = gsl_histogram_max_bin(hist);
	
	/* Around that value will be the range admissible for theta */
	double min_range, max_range;
	gsl_histogram_get_range(hist,max_bin,&min_range,&max_range);
	
	/* Extend the range of the search */
	double extend_range = deg2rad(params->gpm_extend_range_deg);
	min_range += -extend_range;
	max_range += +extend_range;

	/*	if(jf()) fprintf(jf(), "iteration 0\n");
	journal_pose("x_old", u);*/


	/*	if(jf()) fprintf(jf(), "iteration 1\n");
	journal_pose("x_old", u);*/


	/* Now repeat the samples generation with a smaller domain */
	u[2] = 0.5 * (max_range + min_range);
	double new_range_deg = rad2deg( 0.5*(max_range - min_range) );
	
	double x_new[3];
	int num_correspondences=-1;
	ght_one_shot(laser_ref, laser_sens,
			u, params->max_linear_correction*2,
			new_range_deg, interval, x_new, &num_correspondences) ;
			
	if(num_correspondences < laser_ref->nrays) {
		sm_error("sm_gpm(): I found only %d correspondences in the second pass of GPM. I consider it a failure.\n",
			num_correspondences);
		return;
	}

	/* Et voila, in x_new we have the answer */

	{
		sm_debug("gpm : max_correction_lin %f def %f\n", params->max_linear_correction, 		params->max_angular_correction_deg);
		sm_debug("gpm : acceptable range for theta: [%f, %f]\n", min_range,max_range);
		sm_debug("gpm : 1) Num correspondences for theta: %d\n", num_correspondences_theta);

		sm_debug("gpm 1/2: new u = : %s \n", friendly_pose(u) );
		sm_debug("gpm 1/2: New range: %f to %f\n",rad2deg(min_range),rad2deg(max_range));

		sm_debug("gpm 2/2: Solution: %s \n", friendly_pose(x_new));
	/*	if(jf()) fprintf(jf(), "iteration 2\n");
		journal_pose("x_old", x_new);	*/
	}

	/* Administrivia */

	res->valid = 1;
	copy_d(x_new, 3, res->x);
	
	res->iterations = 0;
	
	gsl_histogram_free(hist);
}

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
	const double*x0, double max_linear_correction,
	double max_angular_correction_deg, int interval, gsl_histogram*hist, int*num_correspondences) 
{
	/** Compute laser_sens's points in laser_ref's coordinates by roto-translating by x0 */
	ld_compute_world_coords(laser_sens, x0);
	
	int count = 0;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		if(i % interval) continue;
		
		const double * p_i = laser_sens->points[i].p;
		
		const double * p_i_w = laser_sens->points_w[i].p;
		int from; int to; int start_cell;
		possible_interval(p_i_w, laser_ref, max_angular_correction_deg,
			max_linear_correction, &from, &to, &start_cell);

//		printf("\n i=%d interval = [%d,%d] ", i, from, to);
		int j;
		for(j=from;j<=to;j++) {
			if(!laser_ref->alpha_valid[j]) continue;
			if(j % interval) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			double theta_diff = angleDiff(theta,x0[2]); 
			if( fabs(theta_diff) > deg2rad(max_angular_correction_deg) )
				continue;
			theta = x0[2] + theta_diff; // otherwise problems near +- PI
	
			const double * p_j = laser_ref->points[j].p;
			
			double c = cos(theta); double s = sin(theta);
			double t_x = p_j[0] - (c*p_i[0]-s*p_i[1]);
			double t_y = p_j[1] - (s*p_i[0]+c*p_i[1]);
			double t_dist = sqrt( square(t_x-x0[0]) + square(t_y-x0[1]) );

			if(t_dist > max_linear_correction)
				continue;
				
			/*double weight = 1/(laser_sens->cov_alpha[i]+laser_ref->cov_alpha[j]);*/
			double weight = 1;
			gsl_histogram_accumulate(hist, theta, weight);
			gsl_histogram_accumulate(hist, theta+2*M_PI, weight); /* be robust */
			gsl_histogram_accumulate(hist, theta-2*M_PI, weight);
			count ++;
		}
	}
	*num_correspondences = count;
	sm_debug(" correspondences = %d\n",count);
}

void ght_one_shot(LDP laser_ref, LDP laser_sens,
		const double*x0, double max_linear_correction,
	double max_angular_correction_deg, int interval, double*x, int*num_correspondences) 
{
	/** Compute laser_sens's points in laser_ref's coordinates by roto-translating by x0 */
	ld_compute_world_coords(laser_sens, x0);
	
	double L[3][3]  = {{0,0,0},{0,0,0},{0,0,0}};
	double z[3] = {0,0,0};
	
	int count = 0;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		if(i % interval) continue;
		

		const double * p_i = laser_sens->points_w[i].p;

		const double * p_i_w = laser_sens->points_w[i].p;
		int from; int to; int start_cell;
		possible_interval(p_i_w, laser_ref, max_angular_correction_deg,
			max_linear_correction, &from, &to, &start_cell);
//		from = 0; to = laser_ref->nrays-1;
		

		int j;
		for(j=from;j<=to;j++) {
			if(j % interval) continue;
			if(!laser_ref->alpha_valid[j]) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			double theta_diff = angleDiff(theta,x0[2]); 
			if( fabs(theta_diff) > deg2rad(max_angular_correction_deg) )
				continue;
			theta = x0[2] + theta_diff; // otherwise problems near +- PI
	
			const double * p_j = laser_ref->points[j].p;
			
			double c = cos(theta); double s = sin(theta);
			double t_x = p_j[0] - (c*p_i[0]-s*p_i[1]);
			double t_y = p_j[1] - (s*p_i[0]+c*p_i[1]);
			double t_dist = sqrt( square(t_x-x0[0]) + square(t_y-x0[1]) );

			if(t_dist > max_linear_correction)
				continue;

			/*double weight = 1/(laser_sens->cov_alpha[i]+laser_ref->cov_alpha[j]);
			double weight = exp( -square(t_dist) - 5 * square(theta-x0[2]) );*/
			
			double weight = 1;
			
			double alpha = laser_ref->alpha[j];
			double ca = cos(alpha); double sa=sin(alpha);

//				printf("%d ", (int) rad2deg(theta));

/*			printf("valid %d alpha %f weight %f t_x %f t_y %f\n",
				laser_ref->alpha_valid[j],alpha,weight,
				t_x, t_y); */
			z[0] += weight*(ca*ca*t_x + sa*ca*t_y);
			z[1] += weight*(sa*ca*t_x + sa*sa*t_y);
			z[2] += weight*theta;
			L[0][0] += weight* ca * ca;
			L[0][1] += weight* sa * ca;
			L[1][0] += weight* sa * ca;
			L[1][1] += weight* sa * sa;
			L[2][2] += weight;
			
			count += 1;
		}
	}
	
	*num_correspondences = count;

	if(1) {
		
		double weight = 0.5 * count;
		z[0] += x0[0] * weight;
		z[1] += x0[1] * weight;
		L[0][0] += weight;
		L[0][1] += 0;
		L[1][0] += 0;
		L[1][1] += weight;
	}
	

	egsl_push();
		val eL = egsl_alloc(3,3);
			size_t a,b; 
			for(a=0;a<3;a++) 
			for(b=0;b<3;b++) 
				*egsl_atmp(eL,a,b) = L[a][b];

/*		egsl_print("eL", eL);*/
		val ez = egsl_vFa(3,z);
		
		val ex = m(inv(eL), ez);
		
		egsl_v2a(ex, x);
		

/*		egsl_print("eL", eL);
		egsl_print("ez", ez);
		egsl_print("ex", ex); */

	egsl_pop();

//	sm_debug("gpm: second step: theta = %f   %f / %d = %f \n", rad2deg(x[2]), rad2deg(z[2]), count, rad2deg(z[2]) / count);
	sm_debug("gpm: second step: found %d correspondences\n",count);
	
}

