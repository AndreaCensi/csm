#include <libgen.h>
#include <math.h>

#include <options/options.h>
#include "../csm/csm_all.h"


struct ld_fisher_params {
	double sigma;
};

val ld_fisher0(LDP ld);

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	struct ld_fisher_params p;
	
	struct option* ops = options_allocate(3);
	options_double(ops, "sigma", &p.sigma, 0.01, 
		"Std deviation of gaussian noise");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "Writes Fisher's information matrix.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	egsl_push();

	LDP ld;
	while( (ld = ld_from_json_stream(stdin))) {
		egsl_push();
			val fisher = egsl_scale( square(p.sigma),  ld_fisher0(ld) );
			
			JO jo = matrix_to_json(egsl_gslm(fisher));
			printf("%s\n", json_object_to_json_string(jo));
			jo_free(jo);
		egsl_pop();
		ld_free(ld);
	}

	egsl_pop();
	
	return 0;
}
