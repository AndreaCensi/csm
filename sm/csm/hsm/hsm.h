#ifndef H_CSM_HSM
#define H_CSM_HSM


/** Public interface */

struct hsm_params {
	double max_norm;
	double linear_cell_size;
	double angular_cell_size_deg;
		
	int num_angular_hypotheses;
	int num_linear_hypotheses;
	
	/* Minimum distance between angular hypotheses */
	double angular_hyp_min_distance_deg;
	int xc_ndirections;
	double xc_directions_min_distance_deg;

	double max_translation;
};

struct hsm_hypothesis {
	double pose[3];
	double quality;	
};

struct hsm_result {
	int nhypotheses;
	struct hsm_hypothesis* hypotheses;
	
	double elapsed_time;
};

typedef struct hsm_buffer_struct* hsm_buffer;

/** Allocates the buffer structures. Remember to call hsm_buffer_free afterwards */
hsm_buffer hsm_buffer_alloc(struct hsm_params*);
void hsm_buffer_free(hsm_buffer);



void hsm_compute_ht_point(hsm_buffer, double x, double y, double weight);

/** Computes the spectrum in the buffer */
void hsm_compute_spectrum(hsm_buffer);

void hsm_find_local_maxima_circ(int n, const double*f, int*maxima, int*nmaxima);
int hsm_is_angle_between_smaller_than_deg(double angle1, double angle2, double threshold_deg);


void hsm_match(struct hsm_params*p, hsm_buffer b1, hsm_buffer b2);

/** Private interface */

void hsm_fill_options(struct hsm_params*);

struct hsm_buffer_struct {
	int num_linear_cells;
	int num_angular_cells;	
	double rho_min, rho_max;
	
	/** ht[theta][rho] */
	double *theta;
	double **ht;
	double *hs;
	
	/* Used during computation of HT */
	double *sint,*cost;
	
	/* Used during matching */
	double *hs_cross_corr;
};


/** Returns 0 if out of the buffer. rho_index is the closest cell, 
	alpha between -0.5 and 0.5 specifies the distance from the center of the cell. */
		int hsm_rho2index(hsm_buffer b, double rho, int *rho_index, double *alpha);


void hsm_circular_cross_corr_stupid(const double *f1, const double *f2, double*res);
void hsm_find_peaks_circ(int n, const double*f, double min_angle_deg, int unidir, int max_peaks, 
	int*peaks, int* npeaks) ;

void swap_int(int*a,int*b);
void quicksort_with_indexes(const double*values, int*array, int begin, int end);

#endif
