#ifndef H_LASER_DATA_JSON
#define H_LASER_DATA_JSON

#include "laser_data.h"
#include "algos.h"

#include <json-c/json.h>

JO matrix_to_json(gsl_matrix*m);
JO vector_to_json(gsl_vector*v);

JO corr_to_json(struct correspondence*, int n);
int json_to_corr(JO jo, struct correspondence*, int n);

JO ld_to_json(LDP);
LDP json_to_ld(JO);

JO result_to_json(struct sm_params*p, struct sm_result *r);
LDP ld_from_json_stream(FILE*);

/** 
	Tries to read a laser scan from file. If error or EOF, it returns 0.
	Whitespace is skipped. If first valid char is '{', it tries to read 
	it as JSON. If next char is 'F' (first character of "FLASER"),
	it tries to read in Carmen format. Else, 0 is returned. 
*/
LDP ld_read_smart(FILE*);

#endif
