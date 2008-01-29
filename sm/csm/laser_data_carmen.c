#include <string.h>
#include <math.h>
#include <ctype.h>

#include "csm_all.h"

const char * carmen_prefix = "FLASER ";

/** Returns 0 on success */
int read_next_double(const char*line, size_t*cur, double*d);

/** Returns 0 on success */
int read_next_integer(const char*line, size_t*cur, int*d);

/** Always returns 0 */
int read_next_string(const char*line, size_t*cur, char*buf, size_t buf_len);


/** Returns 0 on success */
int read_next_double(const char*line, size_t*cur, double*d) {
	int inc;
	int ret = sscanf(line+*cur, " %lf %n", d, &inc);
	if(1 != ret) {
		sm_error("Could not read double at %p + %d '%s'. ret: %d.\n", line, *cur, line+*cur, ret);
		return -1;
	}
	*cur += inc;
	return 0;
}

/** Returns 0 on success */
int read_next_integer(const char*line, size_t*cur, int*d) {
	int inc;
	if(1 != sscanf(line+*cur, " %d %n", d, &inc)) {
	/*	sm_error("Could not read integer.\n");*/
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

LDP ld_from_carmen_string(const char*line) {
	
	if(0 != strncmp(line, carmen_prefix, strlen(carmen_prefix))) {
		sm_error("This is not a Carmen line: \n-> %s\n", line);
		return 0;
	}
	
	size_t cur = strlen(carmen_prefix); 

	
	int nrays=-1;
	if(read_next_integer(line, &cur, &nrays)) {
		sm_error("Could not get number of rays.\n");
		goto error;
	}

	LDP ld = ld_alloc_new(nrays);
	
	
	double fov = M_PI;
	double min_reading = 0;
	double max_reading = 80;
	
	if(nrays == 769) {
		min_reading = 0.001;
		max_reading = 4;
		fov = deg2rad(270.0);

		static int print = 0;
		if(!print) { print = 1;
			sm_info("Assuming that 769 rays is an Hokuyo "
			 "with fov = %f deg, min_reading = %f m, max_reading = %fm\n",
				rad2deg(fov), min_reading, max_reading);
		}
	}
	
	ld->min_theta = -fov/2;
	ld->max_theta = +fov/2;
	
	int on_error = 0;
	int i;
	for(i=0;i<nrays;i++) {
		double reading;
		if(read_next_double(line,&cur,&reading)) {
			sm_error("Could not read ray #%d / %d, \n", i, nrays); 
			on_error = 1;
			break;
		}
			
		ld->valid[i] = (reading > min_reading) && (reading < max_reading);
		ld->readings[i] = ld->valid[i] ? reading : NAN;
		ld->theta[i] = ld->min_theta + i * 
		  (ld->max_theta-ld->min_theta) / (ld->nrays-1);
		
		/* bad hokuyo!! */
		if(nrays == 769) {
			if(i>725 || i<44) { 
				ld->valid[i] = 0; 
				ld->readings[i] = NAN;
			}
		}
		
	}
	
	if(on_error) goto error;
	
	if(read_next_double(line,&cur,ld->estimate+0)) goto error;
	if(read_next_double(line,&cur,ld->estimate+1)) goto error;
	if(read_next_double(line,&cur,ld->estimate+2)) goto error;
	if(read_next_double(line,&cur,ld->odometry+0)) goto error;
	if(read_next_double(line,&cur,ld->odometry+1)) goto error;
	if(read_next_double(line,&cur,ld->odometry+2)) goto error;

	/* Following: ipc_timestamp hostname timestamp */
	/* Two options:
		double string double: 
			the first is timestamp in seconds, the second is discarded
		int string int:
			the first is sec, the second is usec 
	*/
	static int warn_format = 1;

	int inc; int sec=-1, usec=-1;
	int res = sscanf(line + cur, "%d %s %d%n", &sec, ld->hostname, &usec,  &inc);
	if(3 == res) {
		ld->tv.tv_sec = sec;
		ld->tv.tv_usec = usec;
		if(warn_format)
			sm_info("Reading timestamp as 'sec hostname usec'.\n");
	} else {
		double v1=-1, v2=-1;
		res = sscanf(line + cur, "%lf %s %lf%n", &v1, ld->hostname, &v2,  &inc);
		if(3 == res) {
			ld->tv.tv_sec = (int) floor(v1);
			ld->tv.tv_usec = floor( (v1 - floor(v1)) * 1e6 );
			
			if(warn_format)
				sm_info("Reading timestamp as doubles (discarding second one).\n");
			
		} else {
			ld->tv.tv_sec = 0;
			ld->tv.tv_usec = 0;
			if(warn_format)
				sm_info("I could not read timestamp+hostname; ignoring (I will warn only once for this).\n");
		}
	}

	warn_format = 0;

	fprintf(stderr, "l");
	return ld;
	
	error:
		printf("Malformed line: '%s'\nat cur = %d\n\t-> '%s'\n", line,(int)cur,line+cur);
		return 0;
}

/** Read next FLASER line in file, or NULL on error  */
int ld_read_next_laser_carmen(FILE*file, LDP*ld) {
	*ld = 0;
	#define MAX_LINE_LENGTH 10000
   char line[MAX_LINE_LENGTH];

	while(fgets(line, MAX_LINE_LENGTH-1, file)) {
		if(0 != strncmp(line, carmen_prefix, strlen(carmen_prefix))) {
			sm_debug("Skipping line: \n-> %s\n", line);
			continue;
		}
		
		*ld = ld_from_carmen_string(line);
		if(!*ld) {
			printf("Malformed line? \n-> '%s'", line);
			return 0;
		} else {
			return 1;
		}
	}
	return 1;
}

/** Write the laser data in CARMEN format */
void ld_write_as_carmen(LDP ld, FILE * stream) {
	int i;
	double timestamp;
	if(!ld_valid_fields(ld)) {
		sm_error("Writing bad data to the stream.\n");
	}
	fprintf(stream, "FLASER %d ", ld->nrays);
	for(i=0; i<ld->nrays; i++){
		fprintf(stream, "%g ", ld->readings[i]);
	}
	fprintf(stream, "%g %g %g ", ld->estimate[0], ld->estimate[1], ld->estimate[2]);
	fprintf(stream, "%g %g %g ", ld->odometry[0], ld->odometry[1], ld->odometry[2]);
	
	timestamp = ld->tv.tv_sec + ((double)ld->tv.tv_sec)/1e6;
	
	fprintf(stream, "%g %s %g", timestamp, ld->hostname, timestamp);
	
	fputs("\n", stream);
}

void ld_write_format(LDP ld, FILE*f, const char * out_format) {
	if(!strncmp(out_format, "carmen", 6))
		ld_write_as_carmen(ld, f);
	else
		ld_write_as_json(ld, f);
	/* XXX: check validity of format string */
}




