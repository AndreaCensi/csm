#include <time.h>
#include <string.h>
#include <libgen.h>
#include <options/options.h>
#include <csm/csm_all.h>


void ld_write_as_json(LDP ld, FILE * stream) {
	JO jo = ld_to_json(ld);
	fputs(json_object_to_json_string(jo), stream);
	fputs("\n", stream);
	jo_free(jo);
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

int main(int argc, const char*argv[]) {
	sm_set_program_name(basename(argv[0]));
	
	/* Read first scan */
	LDP laser_ref, laser_sens;
	
	if(!(laser_ref = ld_read_smart(stdin))) {
		sm_error("Could not read first scan.\n");
		return -1;
	}
	
	ld_write_as_json(laser_ref, stdout);
	int count = 1;
	
	while( (laser_sens = ld_read_smart(stdin)) ) {

		if(ld_equal_readings(laser_ref, laser_sens, 0.00001)) {
			sm_debug("Found double (scan #%d, #%d)\n", count-1, count);
		} else {
			ld_write_as_json(laser_sens, stdout);
		}
		
		ld_free(laser_ref); laser_ref = laser_sens;
		count ++;
	}
	ld_free(laser_ref);
	
	return 0;
}
