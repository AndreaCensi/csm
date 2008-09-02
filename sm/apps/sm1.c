#include <time.h>
#include <string.h>

#include <csm/csm_all.h>

#include <options/options.h>

struct sm1_params {
	const char * file1;
	const char * file2;
	const char * file_jj;
	const char * file_output;
	int debug;
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
	options_string(ops, "out", &p.file_output, "stdout", "Output file (JSON structs)");

	options_int(ops, "debug", &p.debug, 0, "Shows debug information");

	
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	sm_debug_write(p.debug);

	FILE * file1 = open_file_for_reading(p.file1);
	FILE * file2 = !strcmp(p.file1, p.file2) ? file1 
		: open_file_for_reading(p.file2);
	if(!file1 || !file2) return -1;
	
	
	FILE * out = open_file_for_writing(p.file_output);
	if(!out) return -2;
	
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

		/* TODO: add switch */
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
			
		fputs(json_object_to_json_string(jo), out);
		fputs("\n",out);
		
		jo_free(jo);
		ld_free(ld1);
		ld_free(ld2);
	}
	
	fclose(file1);
	fclose(file2);
	fclose(out);
	
	
	return 0;
}
