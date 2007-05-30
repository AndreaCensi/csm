#include <errno.h>
#include <string.h>
#include <math.h>

#include "logging.h"

#include "laser_data_json.h"

JO matrix_to_json(gsl_matrix*m) {
	JO jo = json_object_new_array();
	if(m->size1>1) {
		size_t i,j;
		for(i=0;i<m->size1;i++) {
			JO row  = json_object_new_array();
			for(j=0;j<m->size2;j++) {
				double v = gsl_matrix_get(m,i,j);
				json_object_array_add(row, jo_double_or_null(v));
			}
			json_object_array_add(jo, row);
		}
	} else {
		size_t i = 1, j;
		JO row  = jo;
		for(j=0;j<m->size2;j++) {
			double v = gsl_matrix_get(m,i,j);
			json_object_array_add(row, jo_double_or_null(v));
		}
	}
	return jo;
}

JO corr_to_json(struct correspondence*corr, int n) {
	JO jo = json_object_new_array();
	int i;
	for(i=0;i<n;i++) {
		if(corr[i].valid) {
			JO c = json_object_new_object();
			jo_add(c, "j1", jo_new_int(corr[i].j1));
			jo_add(c, "j2", jo_new_int(corr[i].j2));
			json_object_array_add(jo, c);
		} else {
			json_object_array_add(jo, jo_new_null());
		}
	}
	return jo;
}


JO vector_to_json(gsl_vector*vec) {
	JO jo = json_object_new_array();
	size_t j;
	for(j=0;j<vec->size;j++) {
		double v = gsl_vector_get(vec,j);
		json_object_array_add(jo, jo_double_or_null(v));
	}
	return jo;
}

JO result_to_json(struct sm_params*p, struct sm_result *r) {
	JO jo = json_object_new_object();
	jo_add(jo, "x",  json_double_array(r->x, 3));
	
	if(p->do_compute_covariance) {
		jo_add(jo, "cov_x",  matrix_to_json(r->cov_x_m) );
		jo_add(jo, "dx_dy1",  matrix_to_json(r->dx_dy1_m) );
		jo_add(jo, "dx_dy2",  matrix_to_json(r->dx_dy2_m) );
	}
	jo_add(jo, "iterations", jo_new_int(r->iterations));
	jo_add(jo, "nvalid", jo_new_int(r->nvalid));
	jo_add(jo, "error", jo_new_double(r->error));
	return jo;
}

int is_all_nan(const double *v, int n ) {
	int i; for(i=0;i<n;i++) if(v[i]==v[i]) return 0;
	return 1;
}

/** Adds unless it's all NAN */
void jo_add_double_array(JO jo, const char*name, const double *v, int n) {
	if(is_all_nan(v,n)) return;
	json_object_object_add(jo, (char*)name, json_double_array(v, n));
}
/** true if all values are equal to v */
int all_is(int *a, int n, int v) {
	int i; for(i=0;i<n;i++) if(a[i]!=v) return 0;
	return 1;
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
	if(!all_is(ld->cluster, n, -1))
	json_object_object_add(jo, "cluster",   json_int_array(ld->cluster, n));

	jo_add_double_array(jo, "alpha",        ld->alpha, n);
	jo_add_double_array(jo, "cov_alpha",    ld->cov_alpha, n);
	if(!all_is(ld->alpha_valid, n, 0))
	json_object_object_add(jo, "alpha_valid",  json_int_array(ld->alpha_valid, n));
	jo_add_double_array(jo, "true_alpha",      ld->true_alpha, n);
	jo_add_double_array(jo, "true_alpha_abs",  ld->true_alpha_abs, n);

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

	json_read_double_array(jo, "true_alpha",     ld->true_alpha, n, NAN);
	json_read_double_array(jo, "true_alpha_abs", ld->true_alpha_abs, n, NAN);
	
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



