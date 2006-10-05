#include <gsl/gsl_histogram.h>
#include "icp.h"
#include "math_utils.h"
#include "journal.h"

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
	gsl_vector*x0, double maxLinearCorrection,
	double maxAngularCorrectionDeg, gsl_histogram*hist);
	
void gpm(struct icp_input*params, struct icp_output*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
		
	ld_compute_cartesian(laser_ref);
	ld_simple_clustering(laser_ref, 0.01*5);
	ld_compute_orientation(laser_ref, 4);
	
	ld_compute_cartesian(laser_sens);
	ld_simple_clustering(laser_sens, 0.01*5);
	ld_compute_orientation(laser_sens, 4);
	
	double theta_bin_size = deg2rad(10.0);
	double extend_range = deg2rad(10.0);
	
	size_t nbins = ceil(2*M_PI/theta_bin_size);
	gsl_histogram*hist = gsl_histogram_alloc(nbins);
	gsl_histogram_set_ranges_uniform(hist, -M_PI, M_PI);
	
	gsl_vector * u = vector_from_array(3, params->odometry);
	
	ght_find_theta_range(laser_ref, laser_sens,
		u, params->maxLinearCorrection,
		params->maxAngularCorrectionDeg, hist);
		
 	//gsl_histogram_fprintf(stdout, hist, "%f","%f");
		
	size_t max_bin = gsl_histogram_max_bin(hist);
	
	double min_range, max_range;
	gsl_histogram_get_range(hist,max_bin,&min_range,&max_range);
	
	min_range += -extend_range;
	max_range += +extend_range;
	
	printf("New range: %f to %f\n",rad2deg(min_range),rad2deg(max_range));
	
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);
	
	
	int iteration;
	
	gsl_vector * x_old = gsl_vector_alloc(3);
	gsl_vector * x_new = gsl_vector_alloc(3);
	gsl_vector * delta = gsl_vector_alloc(3);

	res->x[0] = gvg(x_new,0);
	res->x[1] = gvg(x_new,1);
	res->x[2] = gvg(x_new,2);
	
	res->iterations = 0;
	
	gsl_vector_free(x_old);
	gsl_vector_free(x_new);
	gsl_vector_free(delta);
}

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
	gsl_vector*x0, double maxLinearCorrection,
	double maxAngularCorrectionDeg, gsl_histogram*hist) 
{
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		
		gsl_vector * p_i = laser_sens->p[i];

		int from; int to; int start_cell;
		possible_interval(p_i, laser_ref, maxAngularCorrectionDeg,
			maxLinearCorrection, &from, &to, &start_cell);

//		printf("i=%d alpah=%f cov=%f\n", i,laser_sens->alpha[i],laser_sens->cov_alpha[i]);
		int j;
		for(j=from;j<=to;j++) {
			if(!laser_ref->alpha_valid[j]) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			
			if(fabs(theta-gvg(x0,2))>deg2rad(maxAngularCorrectionDeg))
				continue;
	
			gsl_vector * p_j = laser_ref->p[j];
			
			double c = cos(theta); double s = sin(theta);
			double t_x = gvg(p_j,0) - (c*gvg(p_i,0)-s*gvg(p_i,1));
			double t_y = gvg(p_j,0) - (s*gvg(p_i,0)+c*gvg(p_i,1));
			double t_dist = sqrt(square(t_x-gvg(x0,0))+square(t_y-gvg(x0,1)));

			if(t_dist > maxLinearCorrection)
				continue;
				
			double weight = 1/(laser_sens->cov_alpha[i]+laser_ref->cov_alpha[j]);
			gsl_histogram_accumulate(hist,theta, weight);
		}
	}
}

void ght_one_shot(LDP laser_ref, LDP laser_sens,
	gsl_vector*x0, double maxLinearCorrection,
	double maxAngularCorrectionDeg, gsl_vector*x) 
{
	double M[3][3];
	double y[3][1];
	
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		
		gsl_vector * p_i = laser_sens->p[i];

		int from; int to; int start_cell;
		possible_interval(p_i, laser_ref, maxAngularCorrectionDeg,
			maxLinearCorrection, &from, &to, &start_cell);

		int j;
		for(j=from;j<=to;j++) {
			if(!laser_ref->alpha_valid[j]) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			
			if(fabs(theta-gvg(x0,2))>deg2rad(maxAngularCorrectionDeg))
				continue;
	
			gsl_vector * p_j = laser_ref->p[j];
			
			double c = cos(theta); double s = sin(theta);
			double t_x = gvg(p_j,0) - (c*gvg(p_i,0)-s*gvg(p_i,1));
			double t_y = gvg(p_j,0) - (s*gvg(p_i,0)+c*gvg(p_i,1));
			double t_dist = sqrt(square(t_x-gvg(x0,0))+square(t_y-gvg(x0,1)));

			if(t_dist > maxLinearCorrection)
				continue;
				

		}
	}
}

