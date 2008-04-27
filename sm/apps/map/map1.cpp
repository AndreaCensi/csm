#include <string.h>
#include "pan.h"
#include <options/options.h>

struct myparams {
	const char * file_in;
	const char * file_out;
	const char * file_out_stats;
	const char * file_jj;
	int format;
	
	/* which algorithm to run */
	int algo;
} p;

extern "C" void sm_options(struct sm_params*p, struct option*ops);


int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	
	struct sm_params params;
	struct sm_result result;
	
	struct option* ops = options_allocate(100);
	options_string(ops, "in", &p.file_in, "stdin", "Input file ");
	options_string(ops, "out", &p.file_out, "stdout", "Output file ");
	options_string(ops, "out_stats", &p.file_out_stats, "", "Output file (stats) ");
	options_string(ops, "file_jj", &p.file_jj, "",
		"File for journaling -- if left empty, journal not open.");
	options_int(ops, "algo", &p.algo, 0, "Which algorithm to use (0:icp 1:gpm-stripped) ");
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
	if(!ld_valid_fields(laser_ref))  {
		sm_error("Invalid laser data in first scan.\n");
		return -2;
	}
	
	
	/* For the first scan, set estimate = odometry */
	copy_d(laser_ref->odometry, 3, laser_ref->estimate);
	
	ld_write_as_json(laser_ref, file_out);
	int count=-1;
	LDP laser_sens;
	while( (laser_sens = ld_read_smart(file_in)) ) {
		count++;
		if(!ld_valid_fields(laser_sens))  {
			sm_error("Invalid laser data in (#%d in file).\n", count);
			return -(count+2);
		}
		
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
		switch(p.algo) {
			case(0):
				sm_icp(&params, &result); break;
			case(1):
				sm_gpm(&params, &result); break;
			default:
				sm_error("Unknown algorithm to run: %d.\n",p.algo);
				return -1;
		}
		
		/* Add the result to the previous estimate */
		oplus_d(laser_ref->estimate, result.x, laser_sens->estimate);

		/* Write the corrected log */
		ld_write_as_json(laser_sens, file_out);

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
