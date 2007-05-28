#ifndef H_LASER_DATA_JSON
#define H_LASER_DATA_JSON

#include "sm.h"
#include <json.h>

JO ld_to_json(LDP);
LDP json_to_ld(JO);

JO result_to_json(struct sm_params*p, struct sm_result *r);
LDP ld_from_json_stream(FILE*);

#endif