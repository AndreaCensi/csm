#include <string.h>
#include <math.h>
#include "math_utils.h"
#include "laser_data.h"

const char * carmen_prefix = "FLASER ";

/** Returns 0 on success */
int read_next_double(const char*line, int*cur, double*d) {
	int inc;
	if(1 != sscanf(line+*cur, " %lf %n", d, &inc)) {
		printf("Could not read double.\n");
		return -1;
	}
	*cur += inc;
	return 0;
}

/** Read next FLASER line in file (initializes ld). Returns 0 if error or EOF. */
int ld_read_next_laser_carmen(FILE*file, LDP ld) {
	#define MAX_LINE_LENGTH 10000
   char line[MAX_LINE_LENGTH];

	while(fgets(line, MAX_LINE_LENGTH-1, file)) {
		
		if(0 != strncmp(line, carmen_prefix, strlen(carmen_prefix))) {
			printf("Skipping line: \n-> %s\n", line);
			continue;
		}
		
		int cur = strlen(carmen_prefix); int inc;
		
		int nrays;
		if(1 != sscanf(line+cur, "%d %n", &nrays, &inc)) 
			goto error;
		cur += inc;
			
		ld_alloc(ld, nrays);	
		
		ld->min_theta = -M_PI/2;
		ld->max_theta = +M_PI/2;
		
		int i;
		for(i=0;i<nrays;i++) {
			double reading;
			if(read_next_double(line,&cur,&reading)) {
				printf("At ray #%d, ",i); 
				goto error;
			}
				
			ld->valid[i] = reading>0 && reading<80;
			ld->readings[i] = ld->valid[i] ? reading : NAN;
			ld->theta[i] = ld->min_theta+ i * (ld->max_theta-ld->min_theta) / (ld->nrays-1);
		}
		
		if(read_next_double(line,&cur,ld->odometry+0)) goto error;
		if(read_next_double(line,&cur,ld->odometry+1)) goto error;
		if(read_next_double(line,&cur,ld->odometry+2)) goto error;
		if(read_next_double(line,&cur,ld->estimate+0)) goto error;
		if(read_next_double(line,&cur,ld->estimate+1)) goto error;
		if(read_next_double(line,&cur,ld->estimate+2)) goto error;
		
		fprintf(stderr, "l");
		return 0;
		
		error:
			printf("Malformed line? \n-> %s\nat cur = %d\n\t-> %s\n", line,cur,line+cur);
			return -1;
	}
	return -2;
}


