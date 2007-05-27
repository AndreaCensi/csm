#ifndef H_JSON_MORE_UTILS
#define H_JSON_MORE_UTILS

#include <stdio.h>
#include "json_object.h"

typedef struct json_object* JO;

struct json_object* json_read_stream(FILE*);

struct json_object* json_tokener_parse_len(char *str, int len)

	int json_read_double_array(JO s, const char*name, double*p, int n, double when_null);
	int json_read_int_array(JO s, const char*name, int*p, int n, int when_null);

#endif