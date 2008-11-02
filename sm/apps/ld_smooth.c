#include <options/options.h>

#include "../csm/csm_all.h"

struct {
	/** Scale factor */
	double scale_deg; 
	
	/** How many neighbours to consider */
	int neighbours;
} p;

void ld_smooth(LDP ld, int neighbours, double scale_rad);
void convolve(const int*valid,const double*original, int n, double*dest, double*filter, int filter_len);

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	struct option* ops = options_allocate(3);
	options_double(ops, "scale_deg", &p.scale_deg, 0.0, "Scale factor (degrees) ");
	options_int(ops, "neighbours", &p.neighbours, 1, "How many neighbours to consider (regardless of scale).");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple program for smoothing a sensor scan.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	int errors = 0;
	int count = -1;
	LDP ld;
	while( (ld = ld_read_smart(stdin)) ) {
		count++;
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			errors++;
			continue;
		}
		
		ld_smooth(ld, p.neighbours, deg2rad(p.scale_deg) );

		ld_write_as_json(ld, stdout);

		ld_free(ld);
	}
	
	return errors;
}

void convolve(const int*valid,const double*original, int n, double*dest, double*filter, int filter_len) 
{
	int i; /* index on the points */
	int j; /* index on the filter */
	
	for(i=0;i<n;i++) {
		if(!valid[i]) {
			dest[i] = GSL_NAN;
			continue;
		}
		
		dest[i] = 0;
		for(j=-(filter_len-1);j<=(filter_len-1);j++) {
			int i2 = i + j;
			if(i2<0) i2=0; if(i2>=n) i2=n-1;
			if(!valid[i2]) i2 = i; 
			dest[i] += original[i2] * filter[abs(j)];
		}
	}
}

void ld_smooth(LDP ld, int neighbours, double scale_rad) {
	int len = neighbours + 1;
	double filter[len];
	int j;
	static int once = 1;
	for(j=0;j<len;j++) {
		double dist_rad = j * ((ld->max_theta-ld->min_theta)/ld->nrays);
		double mahal = square(dist_rad / scale_rad);
		filter[j] = exp(-mahal);

		if(once) 
			sm_info("filter[%d] = %f mahal = %f dist_rad = %f scale_rad = %f \n", j, filter[j], mahal, dist_rad, scale_rad);
		
	}
	
	once = 0;
	
	/* find total filter */
	double filter_tot = filter[0];
	for(j=1;j<len;j++) filter_tot+=filter[j];
	/* and normalize */
	for(j=0;j<len;j++) filter[j]/=filter_tot;

	double new_readings[ld->nrays];
	convolve(ld->valid, ld->readings, ld->nrays, new_readings, filter, len);
	copy_d(new_readings, ld->nrays,  ld->readings);

	for(j=0;j<len;j++) {
		
	}
}


