#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <options/options.h>

#include <csm/csm_all.h>

void ld_write_as_json(FILE*f, LDP ld) {
	JO jo = ld_to_json(ld);
	fputs(json_object_to_json_string(jo), f);
	fputs("\n", f);
	jo_free(jo);
}

int main(int argc, const char * argv[]) {

	const char*input_filename;
	const char*output_filename;
	double diff[3] = {0, 0, 0};
	
	struct option* ops = options_allocate(10);
	options_string(ops, "in", &input_filename, "stdin", "input file");
	options_string(ops, "out", &output_filename, "stdout", "output file");
	options_double(ops, "x", &(diff[0]), 0.0, "x (m)");
	options_double(ops, "y", &(diff[1]), 0.0, "y (m)");
	options_double(ops, "theta", &(diff[2]), 0.0, "theta (rad)");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, " Corrects bias in odometry.\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * input_stream = open_file_for_reading(input_filename);
	FILE *output_stream = open_file_for_writing(output_filename);
	
	if(!input_stream || !output_stream) return -1;
	
	LDP laser_ref = ld_read_smart(input_stream);
	if(!laser_ref) {
		sm_error("Cannot read first scan.\n");
		return -2;
	}
	
	copy_d(laser_ref->odometry, 3, laser_ref->estimate);

	LDP laser_sens;
	ld_write_as_json(output_stream, laser_ref);
	
	while((laser_sens = ld_read_smart(input_stream))) {
		double guess[3];
		pose_diff_d(laser_sens->odometry, laser_ref->odometry, guess);
		
		oplus_d(guess, diff, guess);
		oplus_d(laser_ref->estimate, guess, laser_sens->estimate);
		
		ld_write_as_json(output_stream, laser_sens);
		
		ld_free(laser_ref);
		laser_ref = laser_sens;
	}
	
	return 0;
}
