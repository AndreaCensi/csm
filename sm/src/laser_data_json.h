#ifndef H_LASER_DATA_JSON
#define H_LASER_DATA_JSON

#include "laser_data.h"
#include <json.h>

typedef struct json_object* JO;

JO ld_to_json(LDP);
LDP json_to_ld(JO);

JO json_read_stream(FILE*);

#endif