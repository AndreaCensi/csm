#ifndef H_LASER_DATA_JSON
#define H_LASER_DATA_JSON

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include <json-c/json.h>
#include <json-c/json_more_utils.h>

#include "laser_data.h"
#include "algos.h"

/* Laserdata to/from json */

JO ld_to_json(LDP);
LDP json_to_ld(JO);

JO corr_to_json(struct correspondence*, int n);
int json_to_corr(JO jo, struct correspondence*, int n);

LDP ld_from_json_stream(FILE*);
LDP ld_from_json_string(const char*s);
void ld_write_as_json(LDP ld, FILE * stream);


/* Other stuff to/from json */

JO matrix_to_json(gsl_matrix*m);
JO vector_to_json(gsl_vector*v);
JO result_to_json(struct sm_params*p, struct sm_result *r);

#endif
