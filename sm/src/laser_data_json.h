#ifndef H_LASER_DATA_JSON
#define H_LASER_DATA_JSON

#include "laser_data.h"
#include <json.h>

JO ld_to_json(LDP);
LDP json_to_ld(JO);

LDP ld_from_json_stream(FILE*);

#endif