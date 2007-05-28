#ifndef H_JSON_MORE_UTILS
#define H_JSON_MORE_UTILS

#include <stdio.h>
#include "json_object.h"

typedef struct json_object* JO;

#define jo_new_double json_object_new_double 
#define jo_new_int    json_object_new_int
#define jo_new_array  json_object_new_array
#define jo_add        json_object_object_add
#define jo_new_null()   0
#define jo_free       json_object_put

JO json_read_stream(FILE*);

JO json_tokener_parse_len(char *str, int len);

int json_read_double_array(JO s, const char*name, double*p, int n, double when_null);
int json_read_int_array(JO s, const char*name, int*p, int n, int when_null);

int json_read_double(JO jo, const char*name, double*p) ;
int json_read_int(JO jo, const char*name, int*p) ;

JO json_double_array(const double *v, int n);
JO json_int_array(const int *v, int n);

#endif