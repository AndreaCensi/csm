#include <string.h>
#include <math.h>
#include <ctype.h>

#include "csm_all.h"

/** Returns 0 on success */
int read_next_double(const char*line, size_t*cur, double*d);

/** Returns 0 on success */
int read_next_integer(const char*line, size_t*cur, int*d);

/** Always returns 0 */
int read_next_string(const char*line, size_t*cur, char*buf, size_t buf_len);

const char * carmen_prefix = "FLASER ";

/** Returns 0 on success */
int read_next_double(const char*line, size_t*cur, double*d) {
	int inc;
	if(1 != sscanf(line+*cur, " %lf %n", d, &inc)) {
		sm_error("Could not read double.\n");
		return -1;
	}
	*cur += inc;
	return 0;
}

/** Returns 0 on success */
int read_next_integer(const char*line, size_t*cur, int*d) {
	int inc;
	if(1 != sscanf(line+*cur, " %d %n", d, &inc)) {
		sm_error("Could not read integer.\n");
		return -1;
	}
	*cur += inc;
	return 0;
}

/** Always returns 0 */
int read_next_string(const char*line, size_t*cur, char*buf, size_t buf_len) {
	int from = *cur; while(isspace(line[from])) from++;
	size_t len = 0; while(!isspace(line[from+len])) len++;
	if(len > buf_len ) len = buf_len;
	strncpy(buf, line+from, len);
	*cur += len;
	return 0;
}

/** Read next FLASER line in file (initializes ld). Returns !=0 if error or EOF. */
int ld_read_next_laser_carmen(FILE*file, LDP ld) {
	#define MAX_LINE_LENGTH 10000
   char line[MAX_LINE_LENGTH];

	while(fgets(line, MAX_LINE_LENGTH-1, file)) {
		
		if(0 != strncmp(line, carmen_prefix, strlen(carmen_prefix))) {
			sm_debug("Skipping line: \n-> %s\n", line);
			continue;
		}
		
		size_t cur = strlen(carmen_prefix); int inc;
		
		int nrays;
		if(1 != sscanf(line+cur, "%d %n", &nrays, &inc)) {
			sm_debug("Could not get number of rays.\n");
			goto error;
		}
		cur += inc;
			
		ld_alloc(ld, nrays);	
		
		double fov = M_PI;
		double min_reading = 0;
		double max_reading = 80;
		
		if(nrays == 769) {
			min_reading = 0.001;
			max_reading = 4;
			fov = deg2rad(270.0);

			static int print = 0;
			if(!print) { print = 1;
				sm_debug("Assuming that 769 rays is an Hokuyo "
				 "with fov = %f deg, min_reading = %f m, max_reading = %fm\n",
					rad2deg(fov), min_reading, max_reading);
			}
		}
		
		ld->min_theta = -fov/2;
		ld->max_theta = +fov/2;
		
		int i;
		for(i=0;i<nrays;i++) {
			double reading;
			if(read_next_double(line,&cur,&reading)) {
				sm_error("Could not read ray #%d, \n",i); 
				goto error;
			}
				
			ld->valid[i] = (reading > min_reading) && (reading < max_reading);
			ld->readings[i] = ld->valid[i] ? reading : NAN;
			ld->theta[i] = ld->min_theta + i * 
			  (ld->max_theta-ld->min_theta) / (ld->nrays-1);
		}
		
		if(read_next_double(line,&cur,ld->estimate+0)) goto error;
		if(read_next_double(line,&cur,ld->estimate+1)) goto error;
		if(read_next_double(line,&cur,ld->estimate+2)) goto error;
		if(read_next_double(line,&cur,ld->odometry+0)) goto error;
		if(read_next_double(line,&cur,ld->odometry+1)) goto error;
		if(read_next_double(line,&cur,ld->odometry+2)) goto error;

		/* Following: ipc_timestamp hostname timestamp */
		char buf[30];
		int sec, usec;
		if(read_next_integer(line, &cur, &sec )) goto error;
		if(read_next_string(line, &cur, buf, 29)) goto error;
		if(read_next_integer(line, &cur, &usec )) goto error;
		
		ld->tv.tv_sec = sec;
		ld->tv.tv_usec = usec;
		
		fprintf(stderr, "l");
		return 0;
		
		error:
			printf("Malformed line? \n-> %s\nat cur = %d\n\t-> %s\n", line,(int)cur,line+cur);
			return -1;
	}
	return -2;
}


