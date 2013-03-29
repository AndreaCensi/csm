#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <time.h>
#include "../csm_all.h"

#include "hsm.h"

hsm_buffer hsm_buffer_alloc(struct hsm_params*p) {
	assert(p->max_norm>0);
	assert(p->linear_cell_size>0);
	assert(p->angular_cell_size_deg>0);
	assert(p->num_angular_hypotheses >0);
	assert(p->linear_xc_max_npeaks>0);
	assert(p->xc_ndirections>0);
	
	hsm_buffer b = (hsm_buffer) malloc(sizeof(struct hsm_buffer_struct));

	b->num_angular_cells = (int) ceil(360.0 / p->angular_cell_size_deg);
	b->num_linear_cells = 1 + 2 * (int) ceil(p->max_norm / p->linear_cell_size);
	b->linear_cell_size = p->linear_cell_size;
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

	b->max_num_results = (int) p->num_angular_hypotheses * pow( (float) p->linear_xc_max_npeaks,  (float) p->xc_ndirections);

	b->num_valid_results = 0;
	b->results = (double**) calloc((size_t)b->max_num_results, sizeof(double*));
	for(int i=0;i<b->max_num_results; i++)
		b->results[i] = (double*) calloc(3, sizeof(double));

	b->results_quality = (double*) calloc((size_t)b->max_num_results, sizeof(double));

	double zero[3] = {0,0,0};
	hsm_compute_ht_base(b, zero);

	return b;
}

void hsm_buffer_free(hsm_buffer b) {
	
	free(b->hs);
	for(int i=0; i<b->num_angular_cells; i++)
		free(b->ht[i]);
	free(b->ht);
	
	free(b->theta);
	free(b->sint);
	free(b->cost);
	
	free(b->hs_cross_corr);
	for(int i=0;i<b->max_num_results; i++)
		free(b->results[i]);
	free(b->results);

	free(b->results_quality);
	free(b);
}

void hsm_compute_ht_base(hsm_buffer b, const double base_pose[3]) {
	b->disp[0] = base_pose[0];
	b->disp[1] = base_pose[1];
	b->disp[2] = base_pose[2];
	b->disp_th_cos = cos(base_pose[2]);
	b->disp_th_sin = sin(base_pose[2]);
}

void hsm_compute_ht_point(hsm_buffer b, double x0, double y0, double weight) {

	double x1 = x0 * b->disp_th_cos - y0 * b->disp_th_sin + b->disp[0];
	double y1 = x0 * b->disp_th_sin + y0 * b->disp_th_cos + b->disp[1];
	
	for(int i=0; i<b->num_angular_cells; i++) {
		double rho = x1 * b->cost[i] + y1 * b->sint[i];
		int rho_index;
		double alpha;
		if(!hsm_rho2index(b, rho, &rho_index, &alpha)) {
			continue;
		}

		b->ht[i][rho_index] += (1-fabs(alpha)) * weight;

		if( (alpha > 0) && (rho_index < b->num_linear_cells - 1))
			b->ht[i][rho_index+1] += (fabs(alpha)) * weight;

		if( (alpha < 0) && (rho_index > 0))
			b->ht[i][rho_index-1] += (fabs(alpha)) * weight;
	}
}

double mdax(double a, double b) {
	return a>b?a:b;
}

/** Returns 0 if out of the buffer. rho_index is the closest cell,
	alpha between -0.5 and 0.5 specifies the distance from the center of the cell. */
int hsm_rho2index(hsm_buffer b, double rho, int *rho_index, double *alpha) {
	*rho_index = 0; *alpha = NAN;
	if ( (rho <= b->rho_min) || (rho >= b->rho_max) )
		return 0;

	/* x belongs to [0, n) */
	double x = b->num_linear_cells * (rho-b->rho_min) / (b->rho_max-b->rho_min);
	
	if(x==b->num_linear_cells) x*=0.99999;
	
	*rho_index = (int) floor( x );
	*alpha = (*rho_index+0.5)-x;

	assert(fabs(*alpha) <= 0.5001);
	assert(*rho_index >= 0);
	assert(*rho_index < b->num_linear_cells);

	return 1;
}


void hsm_compute_spectrum(hsm_buffer b) {
	for(int t=0; t<b->num_angular_cells; t++) {
		b->hs[t] = 0;
		for(int r=0;r<b->num_linear_cells;r++)
			b->hs[t] = max(b->hs[t], b->ht[t][r]);
	}
}

void hsm_compute_spectrum_norm(hsm_buffer b) {
	for(int t=0; t<b->num_angular_cells; t++) {
		b->hs[t] = 0;
		for(int r=0;r<b->num_linear_cells;r++)
			b->hs[t] += b->ht[t][r] * b->ht[t][r];
	}
}

void hsm_match(struct hsm_params*p, hsm_buffer b1, hsm_buffer b2) {
	sm_log_push("hsm_match");
	/* Let's measure the time */
	clock_t hsm_match_start = clock();
	
	assert(b1->num_angular_cells == b2->num_angular_cells);
	assert(p->max_translation > 0);
	assert(b1->linear_cell_size > 0);

	b1->num_valid_results = 0;

	/* Compute cross-correlation of spectra */
	hsm_circular_cross_corr_stupid(b1->num_angular_cells, b2->hs, b1->hs, b1->hs_cross_corr);

	/* Find peaks in cross-correlation */
	int peaks[p->num_angular_hypotheses], npeaks;
	hsm_find_peaks_circ(b1->num_angular_cells, b1->hs_cross_corr, p->angular_hyp_min_distance_deg, 0, p->num_angular_hypotheses, peaks, &npeaks);

	sm_debug("Found %d peaks (max %d) in cross correlation.\n", npeaks, p->num_angular_hypotheses);

	if(npeaks == 0) {
		sm_error("Cross correlation of spectra has 0 peaks.\n");
		sm_log_pop();
		return;
	}

	sm_log_push("loop on theta hypotheses");
	/* lag e' quanto 2 si sposta a destra rispetto a 1 */
	for(int np=0;np<npeaks;np++) {
		int lag = peaks[np];
		double theta_hypothesis = lag * (2*M_PI/b1->num_angular_cells);

		sm_debug("Theta hyp#%d: lag %d, angle %fdeg\n", np, lag, rad2deg(theta_hypothesis));

		/* Superimpose the two spectra */
		double mult[b1->num_angular_cells];
		for(int r=0;r<b1->num_angular_cells;r++)
			mult[r] = b1->hs[r] * b2->hs[pos_mod(r-lag, b1->num_angular_cells)];

		/* Find directions where both are intense */
		int directions[p->xc_ndirections], ndirections;
		hsm_find_peaks_circ(b1->num_angular_cells, b1->hs_cross_corr, p->xc_directions_min_distance_deg, 1, p->xc_ndirections, directions, &ndirections);

		if(ndirections<2) {
			sm_error("Too few directions.\n");
		}
		
		#define MAX_NPEAKS 1024
		assert(p->linear_xc_max_npeaks<MAX_NPEAKS);

		struct {
			/* Direction of cross correlation */
			double angle;
			int nhypotheses;
			struct {
				double delta;
				double value;
			// } hypotheses[p->linear_xc_max_npeaks];
			} hypotheses[MAX_NPEAKS];
		} dirs[ndirections];


		sm_debug("Using %d (max %d) correlations directions.\n", ndirections, p->xc_ndirections);

		int max_lag = (int) ceil(p->max_translation / b1->linear_cell_size);
		int min_lag = -max_lag;
		sm_debug("Max lag: %d cells (max t: %f, cell size: %f)\n",
			max_lag, p->max_translation, b1->linear_cell_size);

		sm_log_push("loop on xc direction");
		/* For each correlation direction */
		for(int cd=0;cd<ndirections;cd++) {

 			dirs[cd].angle =  theta_hypothesis + (directions[cd]) * (2*M_PI/b1->num_angular_cells);

			printf(" cd %d angle = %d deg\n", cd, (int) rad2deg(dirs[cd].angle));

			/* Do correlation */
			int    lags  [2*max_lag + 1];
			double xcorr [2*max_lag + 1];

			int i1 = pos_mod(directions[cd]        , b1->num_angular_cells);
			int i2 = pos_mod(directions[cd] + lag  , b1->num_angular_cells);
			double *f1 = b1->ht[i1];
			double *f2 = b2->ht[i2];

			hsm_linear_cross_corr_stupid(
				b2->num_linear_cells,f2,
				b1->num_linear_cells,f1,
				xcorr, lags, min_lag, max_lag);

			/* Find peaks of cross-correlation */
			int linear_peaks[p->linear_xc_max_npeaks], linear_npeaks;

			hsm_find_peaks_linear(
				2*max_lag + 1, xcorr, p->linear_xc_peaks_min_distance/b1->linear_cell_size,
				p->linear_xc_max_npeaks, linear_peaks, &linear_npeaks);

			sm_debug("theta hyp #%d: Found %d (max %d) peaks for correlation.\n",
				cd, linear_npeaks, p->linear_xc_max_npeaks);

			dirs[cd].nhypotheses = linear_npeaks;
			sm_log_push("Considering each peak of linear xc");
			for(int lp=0;lp<linear_npeaks;lp++) {
				int linear_xc_lag = lags[linear_peaks[lp]];
				double value = xcorr[linear_peaks[lp]];
				double linear_xc_lag_m = linear_xc_lag * b1->linear_cell_size;
				sm_debug("lag: %d  delta: %f  value: %f \n", linear_xc_lag, linear_xc_lag_m, value);
				dirs[cd].hypotheses[lp].delta = linear_xc_lag_m;
				dirs[cd].hypotheses[lp].value = value;
			}
			sm_log_pop();
			
			if(p->debug_true_x_valid) {
				double true_delta = cos(dirs[cd].angle) * p->debug_true_x[0] + 
					sin(dirs[cd].angle) * p->debug_true_x[1];
				sm_debug("true_x    delta = %f \n", true_delta );
			}

		} /* xc direction */
		sm_log_pop();

		sm_debug("Now doing all combinations. How many are there?\n");
		int possible_choices[ndirections];
		int num_combinations = 1;
		for(int cd=0;cd<ndirections;cd++) {
			possible_choices[cd] = dirs[cd].nhypotheses;
			num_combinations *= dirs[cd].nhypotheses;
		}
		sm_debug("Total: %d combinations\n", num_combinations);
		sm_log_push("For each combination..");
		for(int comb=0;comb<num_combinations;comb++) {
			int choices[ndirections];
			hsm_generate_combinations(ndirections, possible_choices, comb, choices);

			/* Linear least squares */
			double M[2][2]={{0,0},{0,0}}; double Z[2]={0,0};
			/* heuristic quality value */
			double sum_values = 0;
			for(int cd=0;cd<ndirections;cd++) {
				double angle = dirs[cd].angle;
				double c = cos(angle), s = sin(angle);
				double w = dirs[cd].hypotheses[choices[cd]].value;
				double y = dirs[cd].hypotheses[choices[cd]].delta;

				M[0][0] += c * c * w;
				M[1][0] += c * s * w;
				M[0][1] += c * s * w;
				M[1][1] += s * s * w;
				Z[0] += w * c * y;
				Z[1] += w * s * y;

				sum_values += w;
			}

			double det = M[0][0]*M[1][1]-M[0][1]*M[1][0];
			double Minv[2][2];
			Minv[0][0] = M[1][1] * (1/det);
			Minv[1][1] = M[0][0] * (1/det);
			Minv[0][1] = -M[0][1] * (1/det);
			Minv[1][0] = -M[1][0] * (1/det);

			double t[2] = {
				Minv[0][0]*Z[0] + Minv[0][1]*Z[1],
				Minv[1][0]*Z[0] + Minv[1][1]*Z[1]};

			/* copy result in results slot */

			int k = b1->num_valid_results;
			b1->results[k][0] = t[0];
			b1->results[k][1] = t[1];
			b1->results[k][2] = theta_hypothesis;
			b1->results_quality[k] = sum_values;
			b1->num_valid_results++;
		}
		sm_log_pop();

	} /* theta hypothesis */
	sm_log_pop();

/*	for(int i=0;i<b1->num_valid_results;i++) {
		printf("#%d %.0fdeg %.1fm %.1fm  quality %f \n",i,
			rad2deg(b1->results[i][2]),
			b1->results[i][0],
			b1->results[i][1],
			b1->results_quality[i]);
	}*/


	/* Sorting based on values */
	int indexes[b1->num_valid_results];
	for(int i=0;i<b1->num_valid_results;i++)
		indexes[i] = i;

	qsort_descending(indexes, (size_t) b1->num_valid_results, b1->results_quality);

	/* copy in the correct order*/
	double*results_tmp[b1->num_valid_results];
	double results_quality_tmp[b1->num_valid_results];
	for(int i=0;i<b1->num_valid_results;i++) {
		results_tmp[i] = b1->results[i];
		results_quality_tmp[i] = b1->results_quality[i];
	}

	for(int i=0;i<b1->num_valid_results;i++) {
		b1->results[i] = results_tmp[indexes[i]];
		b1->results_quality[i] = results_quality_tmp[indexes[i]];
	}

	for(int i=0;i<b1->num_valid_results;i++) {
		char near[256]="";
		double *x = b1->results[i];
		if(p->debug_true_x_valid) {
			double err_th = rad2deg(fabs(angleDiff(p->debug_true_x[2],x[2])));
			double err_m = hypot(p->debug_true_x[0]-x[0],
				p->debug_true_x[1]-x[1]);
			const char * ast = (i == 0) && (err_th > 2) ? "   ***** " : "";
			sprintf(near, "th err %4d  err_m  %5f %s",(int)err_th ,err_m,ast);
		}
		if(i<10)
		printf("after #%d %3.1fm %.1fm %3.0fdeg quality %5.0f \t%s\n",i,
			x[0],
			x[1], rad2deg(x[2]), b1->results_quality[i], near);
	}
	
	
	/* How long did it take? */
	clock_t hsm_match_stop = clock();
	int ticks = hsm_match_stop-hsm_match_start;
	double ctime = ((double)ticks) / CLOCKS_PER_SEC;
	sm_debug("Time: %f sec (%d ticks)\n", ctime, ticks);
	
	sm_log_pop();
}


void hsm_generate_combinations(int nslots, const int possible_choices[],
	int i, int i_choice[])
{
	for(int slot=0;slot<nslots;slot++) {
		i_choice[slot] = i % possible_choices[slot];
		i = (i - i % possible_choices[slot]) / possible_choices[slot];
	}
}

void hsm_find_peaks_circ(int n, const double*f, double min_angle_deg, int unidir, int max_peaks,
	int*peaks, int* npeaks)
{
	sm_log_push("hsm_find_peaks_circ");

	assert(max_peaks>0);

	/* Find all local maxima for the function */
	int maxima[n], nmaxima;
	hsm_find_local_maxima_circ(n, f, maxima, &nmaxima);

	sm_debug("Found %d of %d are local maxima.\n", nmaxima, n);

	/* Sort based on value */
	qsort_descending(maxima, (size_t) nmaxima, f);

	*npeaks = 0;

	sm_log_push("For each maximum");
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

		sm_debug("%saccepting candidate %d; lag = %d value = %f\n",
			acceptable?"":"not ", m, maxima[m], f[maxima[m]]);

		if(acceptable) {
			peaks[*npeaks] = candidate;
			(*npeaks) ++;
		}

		if(*npeaks>=max_peaks) break;
	}
	sm_log_pop();

	sm_debug("found %d (max %d) maxima.\n", *npeaks, max_peaks);
	sm_log_pop();
}


void hsm_find_peaks_linear(int n, const double*f, double min_dist, int max_peaks,
	int*peaks, int* npeaks)
{
	sm_log_push("hsm_find_peaks_linear");

	assert(max_peaks>0);

	/* Find all local maxima for the function */
	int maxima[n], nmaxima;
	hsm_find_local_maxima_linear(n,f,maxima,&nmaxima);

	sm_debug("Found %d of %d are local maxima.\n", nmaxima, n);

	/* Sort based on value */
	qsort_descending(maxima, (size_t) nmaxima,  f);

	*npeaks = 0;
	sm_log_push("for each maximum");
	/* Only retain a subset of these */
	for(int m=0;m<nmaxima;m++) {
		/* Here's a candidate maximum */
		int candidate = maxima[m];
		/* Check that is not too close to the already accepted maxima */
		int acceptable = 1;
		for(int a=0;a<*npeaks;a++) {
			int other = peaks[a];

			if(abs(other-candidate) < min_dist) {
				acceptable = 0; break;
			}
		}

		sm_debug("%s accepting candidate %d; lag = %d value = %f\n",
			acceptable?"":"not", m, maxima[m], f[maxima[m]]);

		if(acceptable) {
			peaks[*npeaks] = candidate;
			(*npeaks) ++;
		}

		if(*npeaks >= max_peaks) break;
	}
	sm_log_pop("");
	sm_debug("Found %d (max %d) maxima.\n", *npeaks, max_peaks);

	sm_log_pop();
}


int hsm_is_angle_between_smaller_than_deg(double angle1, double angle2, double threshold_deg) {
	double dot = cos(angle1)*cos(angle2) + sin(angle1)*sin(angle2);
	return (dot > cos(threshold_deg * M_PI/180));
}

void hsm_find_local_maxima_circ(int n, const double*f, int*maxima, int*nmaxima) {
	*nmaxima = 0;
	for(int i=0;i<n;i++) {
		double val = f[i];
		double left  = f[ pos_mod(i-1,n) ];
		double right = f[ pos_mod(i+1,n) ];
		if( (val>0) && (val>left) && (val>right))
			maxima[(*nmaxima)++] = i;
	}
}


void hsm_find_local_maxima_linear(int n, const double*f, int*maxima, int*nmaxima) {
	*nmaxima = 0;
	for(int i=1;i<n-1;i++) {
		double val = f[i];
		double left = f[i-1];
		double right = f[i+1];
		if( (val>0) && (val>left) && (val>right))
			maxima[(*nmaxima)++] = i;
	}
}



void hsm_circular_cross_corr_stupid(int n, const double *a, const double *b, double*res) {
	/* Two copies of f1 */
	double aa[2*n];
	for(int i=0;i<2*n;i++) aa[i] = a[i%n];
	for(int lag=0;lag<n;lag++) {
		res[lag] = 0;
		for(int j=0;j<n;j++)
			res[lag] += b[j] * aa[j+lag];
	}
}


void hsm_linear_cross_corr_stupid(int na, const double *a, int nb, const double *b, double*res, int*lags, int min_lag, int max_lag) {
	assert(a); 
	assert(b);
	assert(res);
	assert(lags);
	
	for(int l=min_lag;l<=max_lag;l++) {
		lags[l-min_lag] = l;
		
		double r = 0;
		for(int j=0; (j<nb) && (j+l<na);j++) {
			// j + l >= 0
			if(j+l>=0)
			r += b[j] * a[j+l];
		}
	
		res[l-min_lag] = r;

	}
}


const double *qsort_descending_values = 0;

int compare_descending(const void *index_pt1, const void *index_pt2) {
	int i1 = *( (const int*) index_pt1);
	int i2 = *( (const int*) index_pt2);
	const double * f = qsort_descending_values;
	return f[i1] < f[i2] ? +1 : f[i1] == f[i2] ? 0 : -1;
}

void qsort_descending(int *indexes, size_t nmemb, const double*values)
{
	qsort_descending_values = values;
	qsort(indexes, nmemb, sizeof(int), compare_descending);
}



/** Positive modulo */
int pos_mod(int a, int b) {
	return ((a%b)+b)%b;
}



