#include <gsl/gsl_histogram.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>


#include "../csm_all.h"

#include <egsl/egsl_macros.h>

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
		const gsl_vector*x0, double max_linear_correction,
	double max_angular_correction_deg, gsl_histogram*hist);

void ght_one_shot(LDP laser_ref, LDP laser_sens,
		const gsl_vector*x0, double max_linear_correction,
		double max_angular_correction_deg, gsl_vector*x) ;
	
void sm_gpm(struct sm_params*params, struct sm_result*res) {
	LDP laser_ref  = &(params->laser_ref);
	LDP laser_sens = &(params->laser_sens);
		
	ld_compute_cartesian(laser_ref);
	ld_simple_clustering(laser_ref, params->clustering_threshold);
	ld_compute_orientation(laser_ref, params->orientation_neighbourhood, params->sigma);

	ld_compute_cartesian(laser_sens);
	ld_simple_clustering(laser_sens, params->clustering_threshold);
	ld_compute_orientation(laser_sens, params->orientation_neighbourhood, params->sigma);
	
	journal_laser_data("laser_ref",  laser_ref );
	journal_laser_data("laser_sens", laser_sens);

	double theta_bin_size = deg2rad(5.0);
	double extend_range = deg2rad(15.0);
	
	size_t nbins = ceil(2*M_PI/theta_bin_size);
	gsl_histogram*hist = gsl_histogram_alloc(nbins);
	gsl_histogram_set_ranges_uniform(hist, -M_PI, M_PI);
	
	gsl_vector * u = vector_from_array(3, params->first_guess);
	printf("gpm 1/2: old u = : %f %f %f\n",gvg(u,0),gvg(u,1),gvg(u,2));
	
	ght_find_theta_range(laser_ref, laser_sens,
		u, params->max_linear_correction,
		params->max_angular_correction_deg, hist);
		
	if(jf()) gsl_histogram_fprintf(jf(), hist, "%f","%f");
		
	size_t max_bin = gsl_histogram_max_bin(hist);
	
	double min_range, max_range;
	gsl_histogram_get_range(hist,max_bin,&min_range,&max_range);
	
	min_range += -extend_range;
	max_range += +extend_range;

	if(jf()) fprintf(jf(), "iteration 0\n");
	journal_pose("x_old", u);

	gvs(u,2, (max_range+min_range)/2);

	if(jf()) fprintf(jf(), "iteration 1\n");
	journal_pose("x_old", u);


	double newRangeDeg = rad2deg((max_range-min_range)/2);
	
	gsl_vector * x_new = gsl_vector_alloc(3);
	 ght_one_shot(laser_ref, laser_sens,
			u, params->max_linear_correction*2,
			newRangeDeg, x_new) ;

		printf("gpm : max_correction_lin %f def %f\n", params->max_linear_correction, 		params->max_angular_correction_deg);

		printf("gpm 1/2: new u = : %f %f %f\n",gvg(u,0),gvg(u,1),gvg(u,2));
		printf("gpm 1/2: New range: %f to %f\n",rad2deg(min_range),rad2deg(max_range));

		printf("gpm 2/2: Solution: %f %f %f\n",gvg(x_new,0),gvg(x_new,1),gvg(x_new,2));
	
	
	if(jf()) fprintf(jf(), "iteration 2\n");
	journal_pose("x_old", x_new);
	
	
	
	
	gsl_vector * x_old = gsl_vector_alloc(3);
	gsl_vector * delta = gsl_vector_alloc(3);

	res->x[0] = gvg(x_new,0);
	res->x[1] = gvg(x_new,1);
	res->x[2] = gvg(x_new,2);
	
	res->iterations = 0;
	
	gsl_vector_free(x_old);
	gsl_vector_free(u);
	gsl_vector_free(x_new);
	gsl_vector_free(delta);
	gsl_histogram_free(hist);
}

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
	const 	gsl_vector*x0, double max_linear_correction,
	double max_angular_correction_deg, gsl_histogram*hist) 
{
	int count=0;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		
		gsl_vector * p_i = laser_sens->p[i];

		int from; int to; int start_cell;
		possible_interval(p_i, laser_ref, max_angular_correction_deg,
			max_linear_correction, &from, &to, &start_cell);

/*		printf("i=%d alpah=%f cov=%f\n", i,laser_sens->alpha[i],laser_sens->cov_alpha[i]);*/
		int j;
		for(j=from;j<=to;j++) {
			if(!laser_ref->alpha_valid[j]) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			
			if(fabs(theta-gvg(x0,2))>deg2rad(max_angular_correction_deg))
				continue;
	
			gsl_vector * p_j = laser_ref->p[j];
			
			double c = cos(theta); double s = sin(theta);
			double t_x = gvg(p_j,0) - (c*gvg(p_i,0)-s*gvg(p_i,1));
			double t_y = gvg(p_j,1) - (s*gvg(p_i,0)+c*gvg(p_i,1));
			double t_dist = sqrt(square(t_x-gvg(x0,0))+square(t_y-gvg(x0,1)));

		/*	if(i==3) {
				printf(" %f,%f  %d - %d %f <> %f\n",t_x,t_y,i,j,t_dist,max_linear_correction);
			}*/
			if(t_dist > max_linear_correction)
				continue;
				
			double weight = 1/(laser_sens->cov_alpha[i]+laser_ref->cov_alpha[j]);
			weight = 1;
			gsl_histogram_accumulate(hist,theta, weight);
			count ++;
		}
	}
	printf(" correspondences = %d\n",count);
}

void ght_one_shot(LDP laser_ref, LDP laser_sens,
		const gsl_vector*x0, double max_linear_correction,
	double max_angular_correction_deg, gsl_vector*x) 
{
	double L[3][3]  = {{0,0,0},{0,0,0},{0,0,0}};
	double z[3] = {0,0,0};
	
	int count = 0;
	int i;
	for(i=0;i<laser_sens->nrays;i++) {
		if(!laser_sens->alpha_valid[i]) continue;
		
		gsl_vector * p_i = laser_sens->p[i];

		int from; int to; int start_cell;
		possible_interval(p_i, laser_ref, max_angular_correction_deg,
			max_linear_correction, &from, &to, &start_cell);

		int j;
		for(j=from;j<=to;j++) {
			if(!laser_ref->alpha_valid[j]) continue;
			
			double theta = angleDiff(laser_ref->alpha[j], laser_sens->alpha[i]);
			
			if(fabs(theta-gvg(x0,2))>deg2rad(max_angular_correction_deg))
				continue;
	
			gsl_vector * p_j = laser_ref->p[j];
			
			double c = cos(theta); double s = sin(theta);
			double t_x = gvg(p_j,0) - (c*gvg(p_i,0)-s*gvg(p_i,1));
			double t_y = gvg(p_j,1) - (s*gvg(p_i,0)+c*gvg(p_i,1));
			double t_dist = sqrt(square(t_x-gvg(x0,0))+square(t_y-gvg(x0,1)));

			if(t_dist > max_linear_correction)
				continue;

			double weight = 1/(laser_sens->cov_alpha[i]+laser_ref->cov_alpha[j]);

			weight = exp(-square(t_dist)-5*square(theta-gvg(x0,2)));
			double alpha = laser_ref->alpha[j];
			double ca = cos(alpha); double sa=sin(alpha);

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
	
	printf("gpm: second step: found %d correspondences\n",count);
	
	egsl_push();
		val eL = egsl_alloc(3,3);
			size_t a,b; 
			for(a=0;a<3;a++) 
			for(b=0;b<3;b++) 
				*egsl_atmp(eL,a,b) = L[a][b];

		egsl_print("eL", eL);
		val ez = egsl_vFa(3,z);
		
		val ex = m(inv(eL), ez);
		
		egsl_v2vec(ex, x);

		egsl_print("eL", eL);
		egsl_print("ez", ez);
		egsl_print("ex", ex);

	egsl_pop();
	
}

