#include <errno.h>
#include <string.h>
#include <math.h>

#include "logging.h"

#include "laser_data_json.h"

JO result_to_json(struct sm_params*p, struct sm_result *r) {
	JO jo = json_object_new_object();
	jo_add(jo, "x",  json_double_array(r->x, 3));
	
	if(p->do_compute_covariance) {
/*		double cov[9];
		int j;for(j=0;j<9;j++) cov[j] = r->x_cov[(j-j%3)/3][j%3];*/
		jo_add(jo, "cov_x",  json_double_array(r->cov_x, 9));
	}
	jo_add(jo, "iterations", jo_new_int(r->iterations));
	jo_add(jo, "nvalid", jo_new_int(r->nvalid));
	jo_add(jo, "error", jo_new_double(r->error));
	return jo;
}

JO ld_to_json(LDP ld) {
	JO jo = json_object_new_object();
	int n = ld->nrays;
	json_object_object_add(jo, "nrays",     json_object_new_int(ld->nrays));
	json_object_object_add(jo, "min_theta", json_object_new_double(ld->min_theta));
	json_object_object_add(jo, "max_theta", json_object_new_double(ld->max_theta));
	json_object_object_add(jo, "theta",     json_double_array(ld->theta, n));
	json_object_object_add(jo, "readings",  json_double_array(ld->readings, n));
	json_object_object_add(jo, "valid",     json_int_array(ld->valid, n));
	json_object_object_add(jo, "cluster",   json_int_array(ld->cluster, n));

	json_object_object_add(jo, "alpha",     json_double_array(ld->alpha, n));
	json_object_object_add(jo, "cov_alpha",     json_double_array(ld->cov_alpha, n));
	json_object_object_add(jo, "alpha_valid",     json_int_array(ld->alpha_valid, n));

	json_object_object_add(jo, "odometry",     json_double_array(ld->odometry, 3));
	json_object_object_add(jo, "estimate",     json_double_array(ld->estimate, 3));
	return jo;
/*	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;

	gsl_vector**p;
	
	struct correspondence* corr;
*/
}


LDP json_to_ld(JO jo) {
	int n;
	if(!json_read_int(jo, "nrays", &n)) {
		sm_error("Could not read nrays.\n");
		return 0;
	}
	
	LDP ld = ld_alloc_new(n);
	json_read_double(jo, "min_theta", &ld->min_theta);
	json_read_double(jo, "max_theta", &ld->max_theta);	
	json_read_double_array(jo, "theta", ld->theta, n, NAN);	
	json_read_double_array(jo, "readings", ld->readings, n, NAN);	

	json_read_int_array(jo, "valid",     ld->valid, n, 0);
	json_read_int_array(jo, "cluster",   ld->cluster, n, -1);

	json_read_double_array(jo, "alpha",     ld->alpha, n, NAN);
	json_read_double_array(jo, "cov_alpha", ld->cov_alpha, n, NAN);
	json_read_int_array(jo, "alpha_valid",   ld->alpha_valid, n, 0);
	
	json_read_double_array(jo, "odometry", ld->odometry, 3, NAN);
	json_read_double_array(jo, "estimate", ld->estimate, 3, NAN);	
	
	return ld;
}

LDP ld_from_json_stream(FILE*file) {
	JO jo; /* the monkey */
	LDP ld;
	
	jo = json_read_stream(file);
	if(!jo) return 0;

	ld = json_to_ld(jo);
	if(!ld) {
		sm_error("Could not read laser_data:\n\n%s\n", json_object_to_json_string(jo));
		return 0;
	}
	jo_free(jo);
	return ld;
}



