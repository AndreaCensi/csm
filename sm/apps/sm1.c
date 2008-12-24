#include <time.h>
#include <string.h>

#include "../csm/csm_all.h"

#include <options/options.h>

struct sm1_params {
	const char * file1;
	const char * file2;
	const char * file_jj;
	const char * file_output;
	int debug;
	
	int algo;
} p;


extern int distance_counter;
extern void sm_options(struct sm_params*p, struct option*ops);

const char *sm1_banner = 

"There are TWO ways to define the input to this program.\n"
"                                                       \n"
"Say that in file A there are the scans   \n"
"                                                       \n"
"   A1 A2 A3 A4 ...  \n"
"                                                       \n"
"and in file B there are the scans  \n"
"                                                       \n"
"   B1 B2 B3 B4 ...  \n"
"                                                       \n"
"With this command line:  \n"
"                                                       \n"
"    sm1 --file1 A --file2 B  \n"
"                                                       \n"
"the matchings will be:  \n"
"                                                       \n"
"    (A1, B1), (A2, B2), etc.\n"
"                                                       \n"
"While with this command line:\n"
"                                                       \n"
"    sm1 --file1 A --file2 A\n"
"                                                       \n"
"the matchings will be \n"
"                                                       \n"
"    (A1, A2), (A3, A4), (A5, A6), ... \n"
"                                                       \n";
	
	
int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	options_banner(sm1_banner);
	
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
	options_int(ops, "algo", &p.algo, 0, "Which algorithm to use (0:(pl)ICP 1:gpm-stripped 2:HSM) ");

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
		if(!ld1) {
			if(feof(file1)) break;
			sm_error("Invalid data in file1 '%s' \n", p.file1);
			return 2;
		}
		ld2 = ld_from_json_stream(file2);
		if(!ld2) {
			if(feof(file2)) break;
			sm_error("Invalid data in file2 '%s' \n", p.file2);
			return 3;
		}
		
		params.laser_ref = ld1;
		params.laser_sens = ld2;

		pose_diff_d( params.laser_sens->odometry,  
		/* o minus */ params.laser_ref->odometry,
			/* = */ params.first_guess);

		/* Do the actual work */
		switch(p.algo) {
			case(0):
				sm_icp(&params, &result); break;
			case(1):
				sm_gpm(&params, &result); break;
			case(2):
				sm_hsm(&params, &result); break;
			default:
				sm_error("Unknown algorithm to run: %d.\n",p.algo);
				return -1;
		}
		
		
		JO jo = result_to_json(&params, &result);

			double true_x[3];
			pose_diff_d(params.laser_sens->true_pose, 
				params.laser_ref->true_pose, true_x);
			double true_e[3];
			
			
			pose_diff_d(result.x, true_x, true_e);
		/*	int i=0;for(;i<3;i++) true_e[i] = result.x[i] - true_x[i];*/
			
			jo_add_double_array(jo, "true_x", true_x, 3);
			jo_add_double_array(jo, "true_e", true_e, 3);
			
		fputs(json_object_to_json_string(jo), out);
		fputs("\n",out);
		
		jo_free(jo);
		ld_free(ld1);
		ld_free(ld2);
	}
	
	fclose(file1);
	if(file2 != file1) fclose(file2);
	fclose(out);
	
	
	return 0;
}
