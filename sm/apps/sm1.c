#include <time.h>
#include <string.h>

#include <csm/csm_all.h>

#include <options/options.h>

struct sm1_params {
	const char * file1;
	const char * file2;
	const char * file_jj;
} p;


extern int distance_counter;
extern void sm_options(struct sm_params*p, struct option*ops);


int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(100);
	options_string(ops, "file1", &p.file1, "file1.txt",
		"File with first series of scans (at pose1)");
	options_string(ops, "file2", &p.file2, "file2.txt",
		"File with second series of scans (at pose2)");
	options_string(ops, "file_jj", &p.file_jj, "",
		"File for journaling -- if left empty, journal not open.");
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * file1 = open_file_for_reading(p.file1);
	FILE * file2 = !strcmp(p.file1, p.file2) ? file1 
		: open_file_for_reading(p.file2);
	if(!file1 || !file2) return -1;
	
	if(strcmp(p.file_jj, "")) {
		FILE * jj = open_file_for_writing(p.file_jj);
		if(!jj) return -1;
		jj_set_stream(jj);
	}
	
	LDP ld1, ld2;
	while(1) {
		ld1 = ld_from_json_stream(file1);
		ld2 = ld_from_json_stream(file2);
		if(!ld1 || !ld2) break;
		
		params.laser_ref = ld1;
		params.laser_sens = ld2;

		pose_diff_d( params.laser_sens->odometry,  
		/* o minus */ params.laser_ref->odometry,
			/* = */ params.first_guess);

/*			sm_gpm(&params, &result); */
		sm_icp(&params,&result); 
		
		
		JO jo = result_to_json(&params, &result);

			double true_x[3];
			pose_diff_d(params.laser_sens->true_pose, 
				params.laser_ref->true_pose, true_x);
			double true_e[3];
			int i=0;for(;i<3;i++) true_e[i] = result.x[i] - true_x[i];
			jo_add_double_array(jo, "true_x", true_x, 3);
			jo_add_double_array(jo, "true_e", true_e, 3);
			
		puts(json_object_to_json_string(jo));
		puts("\n");
		
		jo_free(jo);
		ld_free(ld1);
		ld_free(ld2);
	}
	
	return 0;
/*
	int num_matchings = 0;
	int num_iterations = 0;
	clock_t start = clock();
	
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
