#include <stdio.h>
#include "journal.h"
#include "math_utils.h"

FILE*f=0; 
int journal_is_open=0;

FILE * jf() { return f;}

void journal_open(const char*file){
	if(journal_is_open) {
		fclose(f);
	}
	
	f=fopen(file, "w");
	/* XXX */
	journal_is_open = f != 0;
}

void write_array_d(int n, double*v) {
	int i; 
	for(i=0;i<n;i++) 
		if(v[i]==GSL_NAN)
			fprintf(f, "nan ");	
		else
			fprintf(f, "%f ", v[i]);	
}

void write_array_i(int n, int*v) {
	int i; for(i=0;i<n;i++) 
		fprintf(f, "%d ", v[i]);	
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
	write_array_i(n,v);
	fprintf(f,"\n");
}

void journal_laser_data(const char*name, struct laser_data*ld) {
	if(!journal_is_open) return;
	fprintf(f, "laser %s nrays %d\n", name, ld->nrays);
	fprintf(f, "laser %s min_theta %f\n", name, ld->min_theta);
	fprintf(f, "laser %s max_theta %f\n", name, ld->max_theta);
	fprintf(f, "laser %s valid ", name);       write_array_i(ld->nrays, ld->valid);
	fprintf(f, "\nlaser %s readings ", name);    write_array_d(ld->nrays, ld->readings);
	fprintf(f, "\nlaser %s cluster ", name);     write_array_i(ld->nrays, ld->cluster);
	fprintf(f, "\nlaser %s alpha_valid ", name); write_array_i(ld->nrays, ld->alpha_valid);
	fprintf(f, "\nlaser %s alpha ", name);       write_array_d(ld->nrays, ld->alpha);
	fprintf(f, "\nlaser %s cov_alpha ", name);   write_array_d(ld->nrays, ld->cov_alpha);
	fprintf(f, "\n");

	journal_write_array_i("down_bigger", ld->nrays, ld->down_bigger );
	journal_write_array_i("down_smaller",ld->nrays, ld->down_smaller);
	journal_write_array_i("up_bigger",   ld->nrays, ld->up_bigger );
	journal_write_array_i("up_smaller",  ld->nrays, ld->up_smaller);	
}

void journal_correspondences(LDP ld) {
	if(!journal_is_open) return;
	fprintf(f, "correspondences ");

	int i;
	for(i=0;i<ld->nrays;i++) {
		int j1 = ld_valid_corr(ld,i) ? ld->corr[i].j1 : -1;
		fprintf(f,"%d ", j1);
	}	
	fprintf(f, "\n");
}

void journal_pose(const char*str, gsl_vector*v) {
	if(!journal_is_open) return;
	fprintf(f, "%s %f %f %f\n", str, gvg(v,0), gvg(v,1), gvg(v,2) );
}

void journal_point(const char*str, gsl_vector*v) {
	if(!journal_is_open) return;
	fprintf(f, "%s %f %f\n", str, gvg(v,0), gvg(v,1) );
}


