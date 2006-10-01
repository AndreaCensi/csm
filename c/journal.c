#include <stdio.h>
#include "journal.h"

FILE*f; 
int journal_is_open=0;

void journal_open(const char*file){
	f=fopen(file, "w");
	// XXX
	journal_is_open = 1;
}

void write_array_d(int n, double*v) {
	int i;
	for(i=0;i<n;i++) {
		fprintf(f, "%f", v[i]);
		if(i!=n-1) fprintf(f, ", ");
	}
}

void journal_write_array_d(const char*str, int n, double*v){
	if(!journal_is_open) return;
	fprintf(f, "%s ", str);
	write_array_d(n,v);
	fprintf(f,"\n");
}

void journal_write_array_i(const char*str, int n, int*v){
	if(!journal_is_open) return;
	fprintf(f, "%s ", str);
	int i;
	for(i=0;i<n;i++) {
		fprintf(f, "%d", v[i]);
		if(i!=n-1) fprintf(f, ", ");
	}
	fprintf(f,"\n");
}

void journal_laser_data(const char*name, struct laser_data*ld) {
	if(!journal_is_open) return;
	fprintf(f, "laser %s nrays %d\n", name, ld->nrays);
	fprintf(f, "laser %s min_theta %f\n", name, ld->min_theta);
	fprintf(f, "laser %s max_theta %f\n", name, ld->max_theta);
	fprintf(f, "laser %s readings ", name);
	write_array_d(ld->nrays, ld->readings);
	fprintf(f, "\n");
}


