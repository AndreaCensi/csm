#include <ctype.h>

#include "csm_all.h"
#include "laser_data_drawing.h"

/** Loads *some* data based on an acceptance criterion. */
int ld_read_some(FILE*file, LDP **array, int*num, int (*accept)(LDP));

	/* Every n scans */
	int interval_accept(LDP ld);
		int interval_count = 0;
		int interval_interval = 10;

	/* Every one */
	int always(LDP ld);
	
	/* Read according to distance */
	int distance_accept(LDP ld);
		static int distance_count;
		static double distance_last_pose[3];
		static double distance_interval_xy = 10;
		static double distance_interval_th = 10;
		static ld_reference distance_reference;
		void distance_accept_reset(ld_reference, double interval_xy, double interval_th);



/* ---------------------------------------- */

int ld_read_some(FILE*file, LDP **array, int*num, int (*accept)(LDP)) {
	*array = 0; *num = 0;
	int size = 10;
	LDP * ar = (LDP*) malloc(sizeof(LDP)*size);
	
	while(1) {
		LDP ld = ld_read_smart(file);

		if(!ld) break;
		
		if( ! (*accept)(ld) ) {
			ld_free(ld);
			continue;
		}
		
		
		ar[(*num)++] = ld;
		
		if(*num > size - 1) {
			size *= 2;
			if(! (ar = (LDP*) realloc(ar, sizeof(LDP)*size)) ) {
				sm_error("Cannot allocate (size=%d)\n", size);
				return 0;
			}
		}
	}

	*array = ar; 

	return feof(file);
}


/* Read every tot scans */
int interval_accept(LDP ld) {
	ld=ld;
	int accept = interval_count % interval_interval == 0;
	interval_count++;
	return accept;
}

int ld_read_some_scans(FILE*file, LDP **array, int*num, int interval) {
	interval_count = 0;
	interval_interval = interval;
	return ld_read_some(file, array, num, interval_accept);
}

/* Read all scans */
int always(LDP ld)  { ld=ld; return 1; }
int ld_read_all(FILE*file, LDP **array, int*num) {
	return ld_read_some(file, array, num, always);
}

void distance_accept_reset(ld_reference which, double interval_xy, double interval_th) {
	distance_count = 0;
	distance_interval_xy = interval_xy;
	distance_interval_th = interval_th;
	distance_reference = which;
}

int distance_accept(LDP ld) {
	double * this_pose = ld_get_reference_pose(ld, distance_reference);
	if(!this_pose) return 0;
	
	distance_count++;
	if(distance_count == 1) {
		copy_d(this_pose, 3, distance_last_pose);
		return 1;
	} else {
		double diff[3];
		pose_diff_d(distance_last_pose, this_pose, diff);
		double distance  = norm_d(diff);
		
		if(distance >= distance_interval_xy || 
		   fabs(diff[2]) >= distance_interval_th ) 
		{
			copy_d(this_pose, 3, distance_last_pose);
		/*	sm_debug("Accepting #%d, %f\n", distance_count, distance);*/
			return 1;
		}
		else 
			return 0;
	}
}

int ld_read_some_scans_distance(FILE*file, LDP **array, int*num, 
	ld_reference which, double d_xy, double d_th) {
	distance_accept_reset(which, d_xy, d_th);
	return ld_read_some(file, array, num, distance_accept);
}



/** 
	Tries to read a laser scan from file. If error or EOF, it returns 0.
	Whitespace is skipped. If first valid char is '{', it tries to read 
	it as JSON. If next char is 'F' (first character of "FLASER"),
	it tries to read in Carmen format. Other lines are discarded.
	0 is returned on error or feof
*/
LDP ld_read_smart(FILE*f) {
	while(1) {
		int c;
		while(1) {
			c = fgetc(f);
			if(feof(f)) { 
				/* sm_debug("eof\n"); */
				return 0;
			}
			if(!isspace(c)) break;
		}
		ungetc(c, f);

		switch(c) {
			case '{': {
/*				sm_debug("Reading JSON\n"); */
				return ld_from_json_stream(f);
			}
			case 'F': {
/*				sm_debug("Reading Carmen\n");  */
				LDP ld;
				if(!ld_read_next_laser_carmen(f, &ld)) {
					sm_error("bad carmen\n");
					return 0;
				}
				return ld;
			}
			default: {
				/*sm_error("Could not read ld. First char is '%c'. ", c);*/
				char max_line[10000];
				char * res = fgets(max_line, 10000-2, f);
				if(!res) {
					sm_error("Could not skip line. \n");
					return 0;
				} else {
					fprintf(stderr, "s");
/*					sm_error("Skipped '%s'\n", res);*/
				}
			}
		}
	}
}

LDP ld_read_smart_string(const char*line) {
	switch(*line) {
		case '{': 
			return ld_from_json_string(line);
		
		case 'F': 
			return ld_from_carmen_string(line);
			
		default:
			sm_error("Invalid laserdata format: '%s'.", line);
			return 0;
	}
}




