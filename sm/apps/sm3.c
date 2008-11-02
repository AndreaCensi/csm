#include <time.h>
#include <string.h>

#include "../csm/csm_all.h"

#include <options/options.h>

struct sm3_params {
	const char * input_filename;
	
} p;
extern void sm_options(struct sm_params*p, struct option*ops);

extern int distance_counter;


int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(100);
	options_string(ops, "in", &p.input_filename, "stdin",
		"Log file");
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * file = open_file_for_reading(p.input_filename);
	if(!file) return -1;
	
	LDP *lds; int lds_size;
	if(!ld_read_all(file, &lds, &lds_size)) {
		sm_error("Cannot read all laser scans.\n");
		return -1;
	}

	sm_debug("Read %d scans.\n", lds_size);


	int num_matchings = 0;
	int num_iterations = 0;
	clock_t start = clock();
	
	int i;
	for(i=0;i<lds_size-1;i++) {
		params.laser_ref = lds[i];
		params.laser_sens = lds[i+1];

		double odometry[3];
		pose_diff_d(params.laser_sens->odometry, params.laser_ref->odometry, odometry);
		double ominus_laser[3], temp[3];
		ominus_d(params.laser, ominus_laser);
		oplus_d(ominus_laser, odometry, temp);
		oplus_d(temp, params.laser, params.first_guess);

		sm_icp(&params,&result); 
		
		num_matchings++;
		num_iterations += result.iterations;
		
		fprintf(stderr, "."); 
	}
	
	clock_t end = clock();
	float seconds = (end-start)/((float)CLOCKS_PER_SEC);
	
	if(num_matchings>0) {
		printf("sm3: CPU time = %f (seconds) (start=%d end=%d)\n", seconds,(int)start,(int)end);
		printf("sm3: Total number of matchings = %d\n", num_matchings);
		printf("sm3: Total number of iterations = %d\n", num_iterations);
		printf("sm3: Avg. iterations per matching = %f\n", num_iterations/((float)num_matchings));
		printf("sm3: Avg. seconds per matching = %f\n", seconds/num_matchings);
		printf("sm3:   that is, %d matchings per second\n", (int)floor(num_matchings/seconds));
		printf("sm3: Avg. seconds per iteration = %f (note: very imprecise)\n", seconds/num_iterations);
		printf("sm3: Number of comparisons = %d \n", distance_counter);
		printf("sm3: Avg. comparisons per ray per iteration = %f \n", 
			(distance_counter/((float)num_iterations*params.laser_ref->nrays)));
	} else {
		sm_error("Empty file?\n");
		return 1;
	}
	return 0;
}
