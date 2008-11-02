#include <options/options.h>
#include "../csm/csm_all.h"

void ld_linearize(LDP ld);
double weighted_mean(double *x, double *weight, int n);

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
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
		
		ld_linearize(ld);

		ld_write_as_json(ld, stdout);

		ld_free(ld);
	}
	
	return errors;
}

double weighted_mean(double *x, double *weight, int n) {
	double sum_weight = 0;
	double sum_x = 0;
	int i;
	for(i=0;i<n;i++) {
		sum_x += weight[i] * x[i];
		sum_weight += weight[i];
	}
	return sum_x / sum_weight;
}


void ld_linearize(LDP ld) {
	int i;
	for(i=0;i<ld->nrays;i++) {
		if(-1 == ld->cluster[i]) continue;
		
		int this_cluster = ld->cluster[i];
		int indexes[ld->nrays];
		int nindexes = 0;
		
		int j;
		for(j=i;j<ld->nrays;j++)
			if(ld->cluster[j]==this_cluster) 
				indexes[nindexes++] = j;
		
		double alpha[nindexes];
		double alpha_weight[nindexes];
		for(j=0;j<nindexes;j++) { 
			alpha[j] = ld->alpha[indexes[j]];
			alpha_weight[j] = 1 / ld->cov_alpha[indexes[j]];
		}
	
		double est_alpha = weighted_mean(alpha, alpha_weight, nindexes);
	
		double rho[nindexes];
		double rho_weight[nindexes];
		for(j=0;j<nindexes;j++) { 
			int i = indexes[j];
			double theta = ld->theta[i];
			double x = cos(theta)  * ld->readings[i];
			double y = sin(theta)  * ld->readings[i];
			rho[j] = cos(est_alpha) * x + sin(est_alpha) * y;
			rho_weight[j] = 1 / ( cos(est_alpha) * cos(theta) 
				+ sin(est_alpha) * sin(theta) );
		}
	
		double est_rho = weighted_mean(rho, rho_weight, nindexes);
	
		for(j=0;j<nindexes;j++) { 
			int i = indexes[j];
			double theta = ld->theta[i];
			ld->readings[i] = est_rho / (cos(est_alpha) * cos(theta) 
				+ sin(est_alpha) * sin(theta));
		}
	
		i = indexes[nindexes-1];
	}
	
}




