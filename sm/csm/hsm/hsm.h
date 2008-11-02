#ifndef H_CSM_HSM
#define H_CSM_HSM

#include <stdlib.h>
/** Public interface */

struct hsm_params {
	/** Max norm for the points (m) (used to define Hough domain) */
	double max_norm;
	/** Size of a linear cell (m)*/
	double linear_cell_size;
	/** Size of an angular cell (deg)*/
	double angular_cell_size_deg;
		
	/** Number of hypotheses for theta */
	int num_angular_hypotheses;
	
	/** Minimum distance between angular hypotheses */
	double angular_hyp_min_distance_deg;
	
	
	/** Number of directions to consider to execute crosscorrelation to find T */
	int xc_ndirections;
	
	/** Minimum distance between said directions */
	double xc_directions_min_distance_deg;


	/** Number of peakks to consider for each linear correlation */
	int linear_xc_max_npeaks;

	/** Minimum distance between said peaks */
	double linear_xc_peaks_min_distance;
	


	double max_translation;

	/** For debugging purpose, set this to 1 and fill next field */
	int debug_true_x_valid;
	/** True result, used for debugging purposes */
	double debug_true_x[3];
};


struct hsm_buffer_struct {
	/** Fields used for computing */
		
		/** Length of hs,theta,sint,cost,etc.*/
		int num_angular_cells;	
		
		/** Num of cells for rho */
		int num_linear_cells;
		
		/** Size of a cell for rho */
		double linear_cell_size;
		
		/** Interval for rho */
		double rho_min, rho_max;

		/** Hough Transform. Access as: ht[theta][rho] */
		double **ht;

		/** Hough Spectrum */
		double *hs;

	/** Results */
	
		/** Size of results array */
		int max_num_results;
		
		/** Number of valid entries in the results array */
		int num_valid_results;

		/** List of poses; theta = results[i][2] */
		double **results;

		/** Quality of results */
		double *results_quality;
	

	/** Private fields */	
		double *theta;

		/** Used during computation of HT */
		double *sint,*cost;
	
		/** Used during matching */
		double *hs_cross_corr;
		
		/** Displacement to be added. See function hsm_compute_ht_base */
		double disp[3];
		double disp_th_cos, disp_th_sin;
};

typedef struct hsm_buffer_struct* hsm_buffer;


	/** Allocates the buffer structures. Remember to call hsm_buffer_free afterwards */
	hsm_buffer hsm_buffer_alloc(struct hsm_params*);
	
	/** Frees the buffer structure */
	void hsm_buffer_free(hsm_buffer);

	/** Adds a point to the Hough Transform */
	void hsm_compute_ht_point(hsm_buffer, double x, double y, double weight);


	void hsm_match(struct hsm_params*p, hsm_buffer b1, hsm_buffer b2);

	/** Private interface */

	/** This adds a base displacement to the point added by hsm_compute_ht_point */
	void hsm_compute_ht_base(hsm_buffer, const double base_pose[3]);
	
	/** Computes the spectrum in the buffer */
	void hsm_compute_spectrum(hsm_buffer);	

	void hsm_compute_spectrum_norm(hsm_buffer b);
	
	/** Finds the local maxima for a circular function. 
	    @maxima is a pointer to a struct of size n 
	    @nmaxima returns the number of maxima found */
	void hsm_find_local_maxima_circ(int n, const double*f, int*maxima, int*nmaxima);
	
	/** Returns true if @angle1 (rad) and @angle2 (rad) are closer than @threshold_deg (degrees!) */
	int hsm_is_angle_between_smaller_than_deg(double angle1, double angle2, double threshold_deg);


	/** Returns 0 if out of the buffer. rho_index is the closest cell, 
	    alpha between -0.5 and 0.5 specifies the distance from the center of the cell. */
	int hsm_rho2index(hsm_buffer b, double rho, int *rho_index, double *alpha);

	/** Creates circular cross-correlation in a stupid way */
	void hsm_circular_cross_corr_stupid(int n, const double *a, const double *b, double*res);
	
	/** Finds the peaks of a circular function @f of length @n. */
	void hsm_find_peaks_circ(int n, const double*f, double min_angle_deg, int unidir, int max_peaks, 
		int*peaks, int* npeaks) ;

	void hsm_find_peaks_linear(int n, const double*f, double min_dist, int max_peaks, 
		int*peaks, int* npeaks);
	void hsm_find_local_maxima_linear(int n, const double*f, int*maxima, int*nmaxima);
	 
	void hsm_linear_cross_corr_stupid(int na, const double *a, int nb, const double *b, double*res, int*lags, int min_lag, int max_lag);

	void hsm_generate_combinations(int nslots, const int possible_choices[], 
		int i, int i_choice[]) ;

	/* a mod b >= 0 */
	int pos_mod(int a, int b);

	/** Sorts the indexes based on the values */
	void qsort_descending(int *indexes, size_t nmemb, const double*values);
	
	/* used by qsort_descending */
	int compare_descending(const void *index_pt1, const void *index_pt2);


#endif
