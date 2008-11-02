#include <math.h>
#include <gsl/gsl_math.h>

#include <options/options.h>

#include "../csm/csm_all.h"

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	

	const char*input_filename;
	const char*output_filename;
	double diff[3] = {0, 0, 0};
	
	double laser[3] ={0,0,0};

	double omega[2] ={0,0};
	double omega_vel = 0;
	double vel[2] ={0,0};
	double vel_omega = 0;
	
	
	struct option* ops = options_allocate(15);
	options_string(ops, "in", &input_filename, "stdin", "input file");
	options_string(ops, "out", &output_filename, "stdout", "output file");
	options_double(ops, "l_x", &(laser[0]), 0.0, "laser x (m)");
	options_double(ops, "l_y", &(laser[1]), 0.0, "laser y (m)");
	options_double(ops, "l_theta", &(laser[2]), 0.0, "laser theta (rad)");

	options_double(ops, "omega0", &(omega[0]), 0.0, "omega (rad)");
	options_double(ops, "omega1", &(omega[1]), 0.0, "omega (linear)");
	options_double(ops, "omega_vel", &(omega_vel), 0.0, "omega x vel");
	options_double(ops, "vel0", &(vel[0]), 0.0, "vel (m)");
	options_double(ops, "vel1", &(vel[1]), 0.0, "vel (linear)");
	options_double(ops, "vel_omega", &(vel_omega), 0.0, "vel x omega");

		
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

	double old_odometry[3];
	copy_d(laser_ref->odometry, 3, old_odometry);
	
	LDP laser_sens;
	ld_write_as_json(laser_ref, output_stream);
	
	while((laser_sens = ld_read_smart(input_stream))) {
		double guess[3], old_guess[3];
		pose_diff_d(laser_sens->odometry, old_odometry, guess);
		pose_diff_d(laser_sens->odometry, old_odometry, old_guess);
		copy_d(laser_sens->odometry, 3, old_odometry);

		if(fabs(guess[2]) > 1e-7)
			guess[2] = old_guess[2] * omega[1] + omega_vel * guess[0] + omega[0];

		if(fabs(guess[0]) > 1e-7)
			guess[0] = old_guess[0] * vel[1] + vel_omega * guess[2] + vel[0];
		
		fprintf(stderr, "odo: %f %f %f Corr: %f rad \t%f m  \n", guess[0], guess[1], guess[2], guess[2]-old_guess[2], guess[0]-old_guess[0]);

		oplus_d(laser_ref->odometry, guess, laser_sens->odometry);
		oplus_d(laser_sens->odometry, laser, laser_sens->estimate);
		
		fprintf(stderr, "ref odo: %s  ref est: %s \n", 
			friendly_pose(laser_ref->odometry),
			friendly_pose(laser_ref->estimate));
		fprintf(stderr, "sens odo: %s  sens est: %s \n", 
			friendly_pose(laser_sens->odometry),
			friendly_pose(laser_sens->estimate));
			
			
			
			ld_write_as_json(laser_sens, output_stream);
		
		ld_free(laser_ref);
		laser_ref = laser_sens;
	}
	
	return 0;
}
