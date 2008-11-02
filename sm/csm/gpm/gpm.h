#ifndef H_GPM_STRIPPED_DOWN
#define H_GPM_STRIPPED_DOWN

#include <gsl/gsl_vector.h>
#include <gsl/gsl_histogram.h>
#include "../csm_all.h"

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
                const double*x0, double max_linear_correction,
        double max_angular_correction_deg, int interval, gsl_histogram*hist, int*num_correspondences);

void ght_one_shot(LDP laser_ref, LDP laser_sens,
                const double*x0, double max_linear_correction,
                double max_angular_correction_deg, int interval, double*x, int*num_correspondences) ;

#endif

