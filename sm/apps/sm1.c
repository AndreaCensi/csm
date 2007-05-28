#include <time.h>
#include <string.h>
#include "../src/math_utils.h"
#include "../src/sm.h"
#include "../src/laser_data.h"
#include "../src/laser_data_json.h"
#include <options/options.h>

struct sm1_params {
	const char * file1;
	const char * file2;
} p;

FILE * open_file(const char*filename) {
	FILE*file = fopen(filename,"r");
	if(file==NULL) {
		fprintf(stderr, "Could not open '%s'.\n", filename); 
		return 0;
	}
	return file;
}

extern void sm_options(struct sm_params*p, struct option*ops);

extern int distance_counter;
int main(int argc, const char*argv[]) {
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(30);
	options_string(ops, "file1", &p.file1, "file1.txt",
		"File with first series of scans (at pose1)");
	options_string(ops, "file2", &p.file2, "file2.txt",
		"File with second series of scans (at pose2)");
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * file1 = open_file(p.file1);
	FILE * file2 = 
		!strcmp(p.file1, p.file2) ? file1 : open_file(p.file2);
	if(!file1 || !file2) return -1;
	
	LDP ld1, ld2;
	while(1) {
		ld1 = ld_from_json_stream(file1);
		ld2 = ld_from_json_stream(file2);
		if(!ld1 || !ld2) break;
		
		params.laser_ref = *ld1;
		params.laser_sens = *ld2;

		gsl_vector *u = gsl_vector_alloc(3);
		gsl_vector *x_old = vector_from_array(3, params.laser_ref.odometry);
		gsl_vector *x_new = vector_from_array(3, params.laser_sens.odometry);
		pose_diff(x_new,x_old,u);
		vector_to_array(u, params.odometry);

/*			sm_gpm(&params,&result); */
		sm_icp(&params,&result); 
		
		JO jo = result_to_json(&params, &result);
		printf("%s\n", json_object_to_json_string(jo));
	}
	
	return 0;
/*
	int num_matchings = 0;
	int num_iterations = 0;
	clock_t start = clock();
	
	if(ld_read_next_laser_carmen(file, &params.laser_ref)) {
		printf("Could not read first scan.\n");
		return -1;
	}
	
	gsl_vector *u = gsl_vector_alloc(3);
	gsl_vector *x_old = gsl_vector_alloc(3);
	gsl_vector *x_new = gsl_vector_alloc(3);
	while(!ld_read_next_laser_carmen(file, &params.laser_sens)) {
		copy_from_array(x_old, params.laser_ref.odometry);
		copy_from_array(x_new, params.laser_sens.odometry);
		pose_diff(x_new,x_old,u);
		vector_to_array(u, params.odometry);
	
	 	sm_gpm(&params,&result);
	 	 sm_icp(&params,&result); 
		
		num_matchings++;
		num_iterations += result.iterations;
	
		ld_dealloc(&(params.laser_ref));
		params.laser_ref = params.laser_sens;
	}
	ld_dealloc(&(params.laser_ref));

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
