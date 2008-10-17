%module sm

%header %{
	#include "rb_sm.h"
%}

struct sm_params {
	LDP laser_ref;
	LDP laser_sens;

	double max_angular_correction_deg;
	double max_linear_correction;
	int max_iterations;
	double epsilon_xy;
	double epsilon_theta; 
	double max_correspondence_dist;
	
	int restart;
	double restart_threshold_mean_error;
	double restart_dt;
	double restart_dtheta;
	
	double sigma;
	
	double clustering_threshold;
	int orientation_neighbourhood;
	
	int do_alpha_test;
	double do_alpha_test_thresholdDeg;
	
	double outliers_maxPerc;
	double outliers_adaptive_order; 
	double outliers_adaptive_mult; 
	
	int do_visibility_test;
	int use_corr_tricks;
	int do_compute_covariance;
	
};

struct sm_result {
	int valid;
	
	double x[3];
	int iterations;
	double error;
	int nvalid;
};


JO ld_to_json(LDP);
LDP json_to_ld(JO);

const char *rb_result_to_json();

LDP string_to_ld(const char*s);
void ld_free(LDP);
void jo_free(JO);

JO json_parse(const char*str);
const char* json_write(JO jo);


int rb_sm_set_configuration(const char*name, const char*value);
void rb_sm_init_journal(const char*journal_file);
void rb_sm_close_journal();


void rb_set_laser_ref(const char*);
void rb_set_laser_sens(const char*);

void rb_sm_odometry(double x, double y, double theta);
void rb_sm_odometry_cov(double cov_x, double cov_y, double cov_theta);

int rb_sm_icp();
int rb_sm_gpm();

void rb_sm_cleanup();

%inline {
extern struct sm_params rb_sm_params;
extern struct sm_result rb_sm_result;
}