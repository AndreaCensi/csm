#include <time.h>

#include <options/options.h>

#include "../csm/csm_all.h"

int ld_equal_readings(LDP ld1, LDP ld2, double epsilon);

int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	
	double epsilon;
	int debug;
	
	
	struct option* ops = options_allocate(3);
	options_double(ops, "epsilon", &epsilon, 0.0001, "minimum difference between rays to be used");
	
	
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}
	
	sm_debug_write(debug);
	
	
	/* Read first scan */
	LDP laser_ref=0, laser_sens;
		
	int count = -1;	
	int num_discarded = 0;
	int num_invalid = 0;
	int ref_index = 0;
	
	while( (laser_sens = ld_read_smart(stdin)) ) {
		count++;
		
		if(!ld_valid_fields(laser_sens))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			num_invalid++;
			continue;
		}
		
		if(!laser_ref) { 
			laser_ref = laser_sens; 
			ld_write_as_json(laser_sens, stdout);
			count++;
			continue;
		}
		
		if(ld_equal_readings(laser_ref, laser_sens, epsilon)) {
			sm_debug("Ignoring scan #%d, too similar to #%d.\n", count, ref_index);
			num_discarded++;
		} else {
			ld_write_as_json(laser_sens, stdout);
			ld_free(laser_ref); 
			laser_ref = laser_sens;
			ref_index = count;
		}
		
	}
	if(laser_ref) ld_free(laser_ref);
	
	sm_info("#   epsilon: %f m\n", epsilon);
	sm_info("#     scans: %d\n", count);
	if(count>0) {
		sm_info("#   invalid: %d\n", num_invalid);
		sm_info("# discarded: %d (%d%%)\n", num_discarded, num_discarded * 100 / count);
	}

	return num_invalid;
}



int ld_equal_readings(LDP ld1, LDP ld2, double epsilon) {
	int i;
	for(i=0;i<ld1->nrays;i++) {
		if(!ld_valid_ray(ld1,i) || !ld_valid_ray(ld2,i)) continue;
		
		if(fabs(ld1->readings[i]-ld2->readings[i]) > epsilon)
			return 0;
	}
	return 1;
}
