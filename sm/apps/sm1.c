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
	int write_post_mortem;
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
	options_int(ops, "write_post_mortem", &p.write_post_mortem, 1, "In case of failure, writes a post mortem.");

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
	int count = 0;
	while(1) {
		count++;
		
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

		if(	any_nan(params.laser_ref->odometry,3) ||  
			any_nan(params.laser_sens->odometry,3) ) {
				sm_error("The 'odometry' field is set to NaN so I don't know how to get an initial guess. I usually use the difference in the odometry fields to obtain the initial guess.\n");
				sm_error(" (file %s) laser_ref->odometry = %s \n", p.file1, friendly_pose(params.laser_ref->odometry) );
				sm_error(" (file %s) laser_sens->odometry = %s \n", p.file2, friendly_pose(params.laser_sens->odometry) );
				sm_error(" I will quit it here. \n");
				return 3;
		}
		
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
		
		if(p.write_post_mortem && !result.valid) {
			char casename[256];
			sprintf(casename, "sm1_failure_matching%d", count);
			sm_error("sm1: matching #%d failed. Writing a special case %s.\n", count, casename );
//			save_testcase(&params)
			
			char file_config[256],file1[256],file2[256],file_jj[256],script[256];
			
			sprintf(file_config, "%s.config", casename);
			sprintf(file1, "%s_laser_ref.json", casename);
			sprintf(file2, "%s_laser_sens.json", casename);
			sprintf(file_jj, "%s.journal", casename);
			sprintf(script, "%s.sh", casename);
			
			FILE * f = fopen(file_config, "w");
			options_dump(ops,f,0);
			fclose(f);

			f = fopen(file1, "w");
			ld_write_as_json(params.laser_ref,f);
			fclose(f);
			
			f = fopen(file2, "w");
			ld_write_as_json(params.laser_sens,f);
			fclose(f);
			
			f = fopen(script, "w");
			fprintf(f, "#!/bin/bash\n");
			fprintf(f, "%s -config %s -file1 %s -file2 %s -debug 1 -file_jj %s -write_post_mortem 0 \n", 
				argv[0], file_config, file1, file2, file_jj);
			fprintf(f, "sm_animate -in %s -out %s_anim.pdf \n", file_jj, casename);
			fclose(f);
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
