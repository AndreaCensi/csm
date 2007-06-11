#ifndef H_GPM_STRIPPED_DOWN
#define H_GPM_STRIPPED_DOWN

#include <gsl/gsl_vector.h>
#include "../csm_all.h"

void ght_find_theta_range(LDP laser_ref, LDP laser_sens,
                const gsl_vector*x0, double max_linear_correction,
        double max_angular_correction_deg, gsl_histogram*hist);

void ght_one_shot(LDP laser_ref, LDP laser_sens,
                const gsl_vector*x0, double max_linear_correction,
                double max_angular_correction_deg, gsl_vector*x) ;

#endif

