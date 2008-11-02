#include <options/options.h>
#include "../csm/csm_all.h"

struct {
	/** Scale factor */
	double scale_deg; 
	
	/** How many neighbours to consider */
	int neighbours;
} p;

void ld_cluster_curv(LDP ld) ;

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
/*	
	struct option* ops = options_allocate(3);
	options_double(ops, "scale_deg", &p.scale_deg, 0.0, "Scale factor (degrees) ");
	options_int(ops, "neighbours", &p.neighbours, 1, "How many neighbours to consider (regardless of scale).");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple program for smoothing a sensor scan.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}
*/	
/* jj_set_stream(open_file_for_writing("ld_cluster_curv.txt")); */

	int errors = 0;
	int count = -1;
	LDP ld;
	while( (ld = ld_read_smart(stdin)) ) {
		count++;
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			return -1;
		}
		
		ld_cluster_curv(ld);

		ld_write_as_json(ld, stdout);

		ld_free(ld);
	}
	
	return errors;
}

void cluster_convolve(const int*cluster,const double*original, int n, double*dest, double*filter, int filter_len, int negate_negative) 
{
	int i; /* index on the points */
	int j; /* index on the filter */
	
	for(i=0;i<n;i++) {
		if(cluster[i] == -1) {
			dest[i] = GSL_NAN;
			continue;
		}
		
		dest[i] = 0;
		for(j=-(filter_len-1);j<=(filter_len-1);j++) {
			int i2 = i + j;
			if(i2<0) i2=0; if(i2>=n) i2=n-1;
			if(cluster[i2] != cluster[i]) i2 = i; 
			double coeff = filter[abs(j)];
			if(j<0 && negate_negative) coeff *= -1;
			dest[i] += original[i2] * coeff;

			if(is_nan(dest[i]))  
				sm_error("i: %d; something wrong after processing i2: %d  cluster[i2]=%d original[i2] = %f \n", i, i2, cluster[i2], original[i2]);
			
		}
		
	}
}

int cluster_find_max(int*cluster, double*v, int n) {
	int i, max = -1;
	for(i=0;i<n;i++) {
		if(cluster[i] == -1) continue;
		if( (max == -1) || (v[i] > v[max]) )
			max = i;
	}
	return max;
}

int find_max(int *v, int n) {
	int i, max = -1;
	for(i=0;i<n;i++) {
		if( (max == -1) || (v[i] > v[max]) )
			max = i;
	}
	return max;	
}

int ld_max_cluster_id(LDP ld) {
	return ld->cluster[ find_max(ld->cluster, ld->nrays)];
}

int ld_cluster_size(LDP ld, int i0) {
	int this_cluster = ld->cluster[i0];
	int num = 0; int i;
	
	for(i=i0;i<ld->nrays;i++)
		if(ld->cluster[i] == this_cluster)
			num++;
		else if(ld->cluster[i] != -1) break;

	return num;
}

void ld_remove_small_clusters(LDP ld, int min_size) {
	int i;
	for(i=0;i<ld->nrays;) {
		int this_cluster = ld->cluster[i];

		if(this_cluster == -1) { i++; continue; }
		int cluster_size = ld_cluster_size(ld, i);

		if(cluster_size < min_size)  {
			for(;i<ld->nrays;i++)
				if(ld->cluster[i] == this_cluster)
					ld->cluster[i] = -1;
				else if(ld->cluster[i] != -1) break;
		} else i++;
	}
}

void ld_mark_cluster_as_invalid(LDP ld, int cluster) {
	int i;
	for(i=0;i<ld->nrays;i++) {
		if(ld->cluster[i] == cluster)
			ld->valid[i] = 0;
	}
}

void array_abs(double*v, int n) {
	int i=0; for(i=0;i<n;i++) v[i] = fabs(v[i]);
}


void ld_cluster_curv(LDP ld) {
	int min_cluster_size = 10;
	double sigma = 0.005; 
	int orientation_neighbours = 4;
	int npeaks = 5;
	double near_peak_threshold = 0.4;

	if(JJ) jj_context_enter("ld_cluster_curv");
	int n = ld->nrays;
	
	
	if(JJ) jj_add_int_array("a00valid", ld->valid, n);
	if(JJ) jj_add_double_array("a01theta", ld->theta, n);
	if(JJ) jj_add_double_array("a02readings", ld->readings, n);
	
	
	ld_simple_clustering(ld, sigma*5);
/*	int i=0; for(i=0;i<n;i++)
		ld->cluster[i] = ld->valid[i] ? 1 : -1;*/
	
	
	if(JJ) jj_add_int_array("a04cluster", ld->cluster, n);
	ld_remove_small_clusters(ld, min_cluster_size);
	ld_mark_cluster_as_invalid(ld, -1);
	if(JJ) jj_add_int_array("a06cluster", ld->cluster, n);
	
	double filter[10] = {.5, .4, .3, .2, .2, .2, .2, .2, .2, .2};
	double deriv_filter[7] = {0, .6, .3, .2, .2, .2, .1};
	double smooth_alpha[n];
	double deriv_alpha[n];

	int p;
	if(JJ) jj_loop_enter("it");
	
	for(p=0;p<npeaks;p++) {  if(JJ) jj_loop_iteration();
		
		if(JJ) jj_add_int_array("cluster", ld->cluster, n);

		ld_compute_orientation(ld, orientation_neighbours, sigma);
		
		int i;
		for(i=0;i<ld->nrays;i++) 
			if(!ld->alpha_valid[i])
			ld->cluster[i] = -1;
		
		if(JJ) jj_add_double_array("alpha", ld->alpha, n);
		cluster_convolve(ld->cluster, ld->alpha, n, smooth_alpha, filter, 10, 0);
		if(JJ) jj_add_int_array("alpha_valid", ld->alpha_valid, n);

		if(JJ) jj_add_double_array("smooth_alpha", smooth_alpha, n);
		cluster_convolve(ld->cluster, smooth_alpha, n, deriv_alpha, deriv_filter, 7, 1);
		if(JJ) jj_add_double_array("deriv_alpha", deriv_alpha, n);
		array_abs(deriv_alpha, n);
		
		int peak = cluster_find_max(ld->cluster, deriv_alpha, n);
		if(JJ) jj_add_int("peak", peak);
		
		int peak_cluster = ld->cluster[peak];
		int up = peak; double threshold = near_peak_threshold  * deriv_alpha[peak];
		while(up<n-1 && (ld->cluster[up]==peak_cluster) && deriv_alpha[up+1] >  threshold) up++;
		int down = peak;
		while(down>1  && (ld->cluster[up]==peak_cluster) && deriv_alpha[down-1] > threshold) down--;
		int j;
		for(j=down;j<=up;j++) {
			ld->cluster[j] = -1;
			ld->valid[j] = 0;
			ld->readings[j] = NAN;
		}
		
		int next_cluster = ld_max_cluster_id(ld) + 1;
		for(j = up+1; j<ld->nrays; j++) {
			if(ld->cluster[j] == peak_cluster)
				ld->cluster[j] = next_cluster;
		}
	}
	if(JJ) jj_loop_exit();

	if(JJ) jj_context_exit();
}


