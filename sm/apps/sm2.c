#include <time.h>
#include <string.h>
#include "../src/math_utils.h"
#include "../src/sm.h"
#include "../src/laser_data.h"
#include "../src/utils.h"
#include "../src/laser_data_json.h"
#include "../src/json_journal.h"
#include "../src/logging.h"
#include <options/options.h>

struct sm1_params {
	const char * file_in;
	const char * file_out;
	const char * file_jj;
	int format;
} p;

extern void sm_options(struct sm_params*p, struct option*ops);

extern int distance_counter;

void spit(LDP ld, FILE * stream) {
	switch(p.format) {
		case(0): {
			JO jo = ld_to_json(ld);
			fputs(json_object_to_json_string(jo), stream);
			fputs("\n", stream);
			jo_free(jo);
		}
		case(1): {
			
		}
	}
}

int main(int argc, const char*argv[]) {
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(30);
	options_string(ops, "in", &p.file_in, "stdin",
		"File with first series of scans (at pose1)");
	options_string(ops, "out", &p.file_out, "stdout",
		"File with second series of scans (at pose2)");
	options_string(ops, "file_jj", &p.file_jj, "",
		"File for journaling -- if left empty, journal not open.");
	options_int(ops, "format", &p.format, 0,
		"Output format (0: json, 1: carmen, ... )");
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * file_in = open_file_for_reading(p.file_in);
	if(!file_in) return -1;
	FILE * file_out = open_file_for_writing(p.file_out);
	if(!file_out) return -1;
	
	if(strcmp(p.file_jj, "")) {
		FILE * jj = open_file_for_writing(p.file_jj);
		if(!jj) return -1;
		jj_set_stream(jj);
	}
	

	LDP laser_ref;
	if(!(laser_ref = ld_read_smart(file_in))) {
		sm_error("Could not read first scan.\n");
		return -1;
	}
	copy_d(laser_ref->odometry, 3, laser_ref->estimate);
	
	int num_matchings = 0;
	int num_iterations = 0;
	clock_t start = clock();
	
	spit(laser_ref, file_out);
	while(1) {
		LDP laser_sens = ld_read_smart(file_in);
		if(!laser_sens) break;
		
		params.laser_ref  = *laser_ref;
		params.laser_sens = *laser_sens;
		pose_diff_d( params.laser_sens.odometry, params.laser_ref.odometry,
			/* = */ params.first_guess);

/*			sm_gpm(&params, &result); */
		sm_icp(&params, &result); 
		
		oplus_d(laser_ref->estimate, result.x, laser_sens->estimate);

		spit(laser_sens, file_out);

		ld_free(laser_ref); laser_ref = laser_sens;
	}
	ld_free(laser_ref);
	
	return 0;
/*
	
	clock_t end = clock();
	float seconds = (end-start)/((float)CLOCKS_PER_SEC);
	
	printf("sm0: CPU time = %f (seconds) (start=%d end=%d)\n", seconds,(int)start,(int)end);
	printf("sm0: Total number of matchings = %d\n", num_matchings);
	printf("sm0: Total number of iterations = %d\n", num_iterations);
	printf("sm0: Avg. iterations per matching = %f\n", num_iterations/((float)num_matchings));
	printf("sm0: Avg. seconds per matching = %f\n", seconds/num_matchings);
	printf("sm0:   that is, %d matchings per second\n", (int)floor(num_matchings/seconds));
	printf("sm0: Avg. seconds per iteration = %f (note: very imprecise)\n", seconds/num_iterations);
	printf("sm0: Number of comparisons = %d \n", distance_counter);
	printf("sm0: Avg. comparisons per ray = %f \n", 
		(distance_counter/((float)num_iterations*params.laser_ref.nrays)));
	
	gsl_vector_free(u);
	gsl_vector_free(x_old);
	gsl_vector_free(x_new);
	return 0;*/
}
