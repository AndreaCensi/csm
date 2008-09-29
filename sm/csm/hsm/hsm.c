#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "hsm.h"


hsm_buffer hsm_buffer_alloc(struct hsm_params*p) {
	hsm_buffer b = (hsm_buffer) malloc(sizeof(struct hsm_buffer_struct));

	b->num_angular_cells = (int) ceil(360.0 / p->angular_cell_size_deg);
	b->num_linear_cells = 1 + 2 * (int) ceil(p->max_norm / p->linear_cell_size);
	b->rho_min = - p->max_norm;
	b->rho_max = + p->max_norm;

	b->hs            =  (double*)  calloc((size_t)b->num_angular_cells, sizeof(double));
	b->hs_cross_corr =  (double*)  calloc((size_t)b->num_angular_cells, sizeof(double));
	b->ht            =  (double**) calloc((size_t)b->num_angular_cells, sizeof(double*));
	
	for(int i=0; i<b->num_angular_cells; i++) {
		b->ht[i] = (double*) calloc((size_t)b->num_linear_cells, sizeof(double));
		for(int r=0;r<b->num_linear_cells;r++)
			b->ht[i][r] = 0;
	}

	b->theta = (double*) calloc((size_t)b->num_angular_cells, sizeof(double));
	b->sint  = (double*) calloc((size_t)b->num_angular_cells, sizeof(double));
	b->cost  = (double*) calloc((size_t)b->num_angular_cells, sizeof(double));
	for(int i=0; i<b->num_angular_cells; i++) {
		b->theta[i] = (2 * M_PI * i) / b->num_angular_cells;
		b->sint[i] = sin(b->theta[i]);
		b->cost[i] = cos(b->theta[i]);
	}
	
	b->hs_cross_corr = (double*) calloc((size_t)b->num_angular_cells, sizeof(double));
	
	return b;
}

void hsm_buffer_free(hsm_buffer b) {
	
}

void hsm_compute_ht_point(hsm_buffer b, double x, double y, double weight) {
	for(int i=0; i<b->num_angular_cells; i++) {
		double rho = x * b->cost[i] + y * b->sint[i];
		int rho_index;
		double alpha;
		if(!hsm_rho2index(b, rho, &rho_index, &alpha)) continue;
	
		b->ht[i][rho_index] += (1-fabs(alpha)) * weight;
		
		if( (alpha > 0) && (rho_index < b->num_linear_cells - 1))
			b->ht[i][rho_index+1] += (fabs(alpha)) * weight;

		if( (alpha < 0) && (rho_index > 0))
			b->ht[i][rho_index-1] += (fabs(alpha)) * weight;
	}
}


/** Returns 0 if out of the buffer. rho_index is the closest cell, 
	alpha between -0.5 and 0.5 specifies the distance from the center of the cell. */
int hsm_rho2index(hsm_buffer b, double rho, int *rho_index, double *alpha) {
	*rho_index = 0; *alpha = NAN; 
	if ( (rho <= b->rho_min) || (rho >= b->rho_max) )
		return 0;

	/* x belongs to [0, n) */
	double x = b->num_linear_cells * (rho-b->rho_min) / (b->rho_max-b->rho_min);
	*rho_index = (int) floor( x );	
	*alpha = (*rho_index+0.5)-x;
	
	assert(fabs(*alpha) < 0.5);
	assert(*rho_index >= 0);
	assert(*rho_index < b->num_linear_cells);
	return 1;
}

void hsm_compute_spectrum(hsm_buffer b) {
	for(int t=0; t<b->num_angular_cells; t++) {
		b->hs[t] = 0;
		for(int r=0;r<b->num_linear_cells;r++)
			b->hs[t] += b->ht[t][r] * b->ht[t][r];
	}
}

void hsm_match(struct hsm_params*p, hsm_buffer b1, hsm_buffer b2) {
	assert(b1->num_angular_cells == b2->num_angular_cells);
	/* Compute cross-correlation */
	hsm_circular_cross_corr_stupid(b1->hs, b2->hs, b1->hs_cross_corr);
	/* Find peaks in cross-correlation */
	int peaks[p->num_angular_hypotheses], npeaks;
	hsm_find_peaks_circ(b1->num_angular_cells, b1->hs_cross_corr, p->angular_hyp_min_distance_deg, 0, p->num_angular_hypotheses, peaks, &npeaks);
	
	for(int np=0;np<npeaks;np++) {
		int lag = peaks[np];
		
		/* Superimpose the two spectrum */
		double mult[b1->num_angular_cells];
		for(int r=0;r<b1->num_angular_cells;r++)
			mult[r] = b1->hs[r] * b2->hs[(r+lag)%b1->num_angular_cells];
			
		/* Find directions where both are intense */
		int directions[p->xc_ndirections], ndirections;
		hsm_find_peaks_circ(b1->num_angular_cells, b1->hs_cross_corr, p->xc_directions_min_distance_deg, 1, p->xc_ndirections, directions, &ndirections);
		
	}
}


void hsm_find_peaks_circ(int n, const double*f, double min_angle_deg, int unidir, int max_peaks, 
	int*peaks, int* npeaks) 
{
	/* Find all local maxima for the function */
	int maxima[n], nmaxima;
	hsm_find_local_maxima_circ(n,f,maxima,&nmaxima);
	
	/* Sort based on value */
	quicksort_with_indexes(f, maxima, 0, nmaxima-1);
	
	*npeaks = 0;
	/* Only retain a subset of these */
	for(int m=0;m<nmaxima;m++) {
		/* Here's a candidate maximum */
		int candidate = maxima[m];
		double candidate_angle = candidate * (2*M_PI/n);
		/* Check that is not too close to the already accepted maxima */
		int acceptable = 1;
		for(int a=0;a<*npeaks;a++) {
			int other = peaks[a];
			double other_angle = other * (2*M_PI/n);
			
			if(hsm_is_angle_between_smaller_than_deg(candidate_angle,other_angle,min_angle_deg)) {
				acceptable = 0; break;
			}
			
			/* If unidir, check also +M_PI */
			if(unidir)
			if(hsm_is_angle_between_smaller_than_deg(candidate_angle+M_PI,other_angle,min_angle_deg)) {
				acceptable = 0; break;
			}
			
		}
		
		if(acceptable) {
			peaks[*npeaks] = candidate;
			*npeaks ++;
		}
	}
}


int hsm_is_angle_between_smaller_than_deg(double angle1, double angle2, double threshold_deg) {
	double dot = cos(angle1)*cos(angle2) + sin(angle1)*sin(angle2);
	return (dot < cos(threshold_deg * M_PI/180));
}
	
void hsm_find_local_maxima_circ(int n, const double*f, int*maxima, int*nmaxima) {
	*nmaxima = 0;
	for(int i=0;i<n;i++) {
		double val = f[i];
		double left = f[(i-1)%n];
		double right = f[(i+1)%n];
		if( (val>0) && (val>left) && (val>right))
			maxima[*nmaxima++] = i;
	}
}

/** Code adapted from Wikipedia */



void swap_int(int*a,int*b) {
	int t = *a; *a = *b; *b=t;
}

void quicksort_with_indexes(const double*values, int*array, int begin, int end) {
	if (end > begin) {
		double pivot = values[array[begin]];
		int l = begin + 1;
		int r = end+1;
		while(l < r) {
			if (values[array[l]] < pivot) {
				l++;
			} else {
				r--;
				swap_int(array+l, array+r); 
			}
		}
		l--;
		swap_int(array+begin, array+l);
		if(l>begin)
			quicksort_with_indexes(values, array, begin, l);
		if(end>r)
			quicksort_with_indexes(values, array, r, end);
	}
}


void hsm_circular_cross_corr_stupid(const double *f1, const double *f2, double*res){
	
}







