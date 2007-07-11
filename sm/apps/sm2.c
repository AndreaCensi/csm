#include <time.h>
#include <string.h>

#include <options/options.h>
#include <csm/csm_all.h>

struct sm1_params {
	const char * file_in;
	const char * file_out;
	const char * file_out_stats;
	const char * file_jj;
	int format;
} p;

extern void sm_options(struct sm_params*p, struct option*ops);

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
	sm_set_program_name(basename(argv[0]));
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(30);
	options_string(ops, "in", &p.file_in, "stdin", "Input file ");
	options_string(ops, "out", &p.file_out, "stdout", "Output file ");
	options_string(ops, "out_stats", &p.file_out_stats, "", "Output file (stats) ");
	options_string(ops, "file_jj", &p.file_jj, "",
		"File for journaling -- if left empty, journal not open.");
	p.format = 0;
/*	options_int(ops, "format", &p.format, 0,
		"Output format (0: log in JSON format, 1: log in Carmen format (not implemented))");*/
	
	sm_options(&params, ops);
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	/* Open input and output files */
	
	FILE * file_in = open_file_for_reading(p.file_in);
	if(!file_in) return -1;
	FILE * file_out = open_file_for_writing(p.file_out);
	if(!file_out) return -1;
	
	if(strcmp(p.file_jj, "")) {
		FILE * jj = open_file_for_writing(p.file_jj);
		if(!jj) return -1;
		jj_set_stream(jj);
	}
	
	FILE * file_out_stats = 0;
	if(strcmp(p.file_out_stats, "")) {
		file_out_stats = open_file_for_writing(p.file_out_stats);
		if(!file_out_stats) return -1;
	}
	
	/* Read first scan */
	LDP laser_ref;
	if(!(laser_ref = ld_read_smart(file_in))) {
		sm_error("Could not read first scan.\n");
		return -1;
	}
	
	/* For the first scan, set estimate = odometry */
	copy_d(laser_ref->odometry, 3, laser_ref->estimate);
	
	
	spit(laser_ref, file_out);
	while(1) {
		LDP laser_sens = ld_read_smart(file_in);
		if(!laser_sens) break;
		
		params.laser_ref  = laser_ref;
		params.laser_sens = laser_sens;

		/* Set first guess as the difference in odometry */
		double odometry[3];
		pose_diff_d(laser_sens->odometry, laser_ref->odometry, odometry);
		double ominus_laser[3], temp[3];
		ominus_d(params.laser, ominus_laser);
		oplus_d(ominus_laser, odometry, temp);
		oplus_d(temp, params.laser, params.first_guess);
		
		/* Do the actual work */
		sm_icp(&params, &result); 
		
		/* Add the result to the previous estimate */
		oplus_d(laser_ref->estimate, result.x, laser_sens->estimate);

		/* Write the corrected log */
		spit(laser_sens, file_out);

		/* Write the statistics (if required) */
		if(file_out_stats) {
			JO jo = result_to_json(&params, &result);
			fputs(jo_to_string(jo), file_out_stats);
			fputs("\n", file_out_stats);
			jo_free(jo);
		}

		ld_free(laser_ref); laser_ref = laser_sens;
	}
	ld_free(laser_ref);
	
	return 0;
}
