#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <options/options.h>

#include <csm/csm_all.h>

struct ld_noise_params {
	int seed;

	double sigma_xy;
	double sigma_theta_deg;
};

int main(int argc, const char * argv[]) {
	struct ld_noise_params p;
	
	struct option* ops = options_allocate(3);
	options_double(ops, "sigma_theta_deg", &p.sigma_theta_deg, 0.0, 
		"Std deviation of gaussian noise for theta (deg) (disabled if 0)");
	options_double(ops, "sigma_xy", &p.sigma_xy, 0.0, 
		"Std deviation of gaussian noise for x,y (disabled if 0)");
	options_int(ops, "seed", &p.seed, 0, 
		"Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple program for adding slip to odometry \n"
			" The 'odometry' field is set to 'true_pose' + noise.\n"
			" Note: this is not applicable to a log, as each scan "
			" in the file is considered separately.\n");
		options_print_help(ops, stderr);
		return -1;
	}
	

	gsl_rng_env_setup();
	gsl_rng * rng = gsl_rng_alloc (gsl_rng_ranlxs0);
	if(p.seed != 0)
	gsl_rng_set(rng, (unsigned int) p.seed);

	LDP ld;
	while( (ld = ld_from_json_stream(stdin))) {
		int i; for(i=0;i<3;i++)
			ld->odometry[i] = ld->true_pose[i];
		
		if(p.sigma_xy > 0) {
			ld->odometry[0] += gsl_ran_gaussian(rng, p.sigma_xy);
			ld->odometry[1] += gsl_ran_gaussian(rng, p.sigma_xy);
		}

		if(p.sigma_theta_deg > 0) {
			ld->odometry[2] += gsl_ran_gaussian(rng, deg2rad(p.sigma_theta_deg));
		}

		JO jo = ld_to_json(ld);
		printf(json_object_to_json_string(jo));
		printf("\n");
		jo_free(jo);
		ld_free(ld);
	}
	
	return 0;
}
