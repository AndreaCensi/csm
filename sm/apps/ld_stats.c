#include <options/options.h>

#include <csm/csm_all.h>

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	const char*input_filename;
	const char*output_filename;
	double max_xy;
	double max_theta_deg;
	
	struct option* ops = options_allocate(10);
	options_string(ops, "in", &input_filename, "stdin", "input file (log)");
	options_string(ops, "out", &output_filename, "stdout", "output file ");
	options_double(ops, "max_xy", &max_xy, 100.0, "Max admissible xy displacement");
	options_double(ops, "max_theta_deg", &max_theta_deg, 360.0, 
		"Max admissible theta displacement (deg)");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info("computes odometry statistics.\n\nOptions:\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * input_stream = open_file_for_reading(input_filename);
	FILE *output_stream = open_file_for_writing(output_filename);
	
	if(!input_stream || !output_stream) return -1;
	
	LDP laser_ref = ld_read_smart(input_stream);
	if(!laser_ref) {
		sm_error("Cannot read first scan.\n.");
		return -2;
	}

	int count = 0, valid = 0;
	LDP laser_sens;
	
	double avg_correction[3] = {0,0,0};
	double max_correction[3] = {0,0,0};
	while((laser_sens = ld_read_smart(input_stream))) {
		double diff_true_pose[3], diff_odometry[3], diff_estimate[3];
		pose_diff_d(laser_sens->odometry, laser_ref->odometry, diff_odometry);
		pose_diff_d(laser_sens->true_pose, laser_ref->true_pose, diff_true_pose);
		pose_diff_d(laser_sens->estimate, laser_ref->estimate, diff_estimate);
		double diff[3];
		pose_diff_d(diff_estimate, diff_odometry, diff);
		
		JO jo = jo_new();
		jo_add_double_array(jo, "diff_odometry", diff_odometry, 3);
		jo_add_double_array(jo, "diff_true_pose", diff_true_pose, 3);
		jo_add_double_array(jo, "diff_estimate", diff_estimate, 3);
		jo_add_double_array(jo, "correction", diff, 3);
		
		fputs(jo_to_string(jo), output_stream);
		fputs("\n", output_stream);
		jo_free(jo);
		ld_free(laser_ref);
		laser_ref = laser_sens;
		
		int use_it = (fabs(diff[0]) < max_xy) && (fabs(diff[1]) < max_xy) &&
			(fabs(diff[2]) < deg2rad(max_theta_deg));
			
		if(use_it) {
			int i; for(i=0;i<3;i++) {
				avg_correction[i] += diff[i];
				max_correction[i] = GSL_MAX(max_correction[i], fabs(diff[i]));
			}
			valid ++;
		}
		
		count ++;
	}

	int i; for(i=0;i<3;i++) 
		avg_correction[i] /= valid;
		
	fprintf(stderr, "Used %d/%d  rays (%.1f %%)\n", valid, count, (100.0 *valid)/count);
	
	fprintf(stderr, "Avg: %f %f %f      = %fdeg \n",
		avg_correction[0], avg_correction[1], avg_correction[2], rad2deg(avg_correction[2]));
	fprintf(stderr, "Max: %f %f %f      = %fdeg \n",
		max_correction[0], max_correction[1], max_correction[2], rad2deg(max_correction[2]));
		
	return 0;
}
