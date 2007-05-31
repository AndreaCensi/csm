#ifndef H_JSON_MORE_UTILS
#define H_JSON_MORE_UTILS

#include <stdio.h>
#include "json_object.h"

typedef struct json_object* JO;

#define jo_new_double json_object_new_double 
#define jo_new_int    json_object_new_int
#define jo_new_array  json_object_new_array
#define jo_new_string json_object_new_string
#define jo_new_null()   0
#define jo_add        json_object_object_add
#define jo_free       json_object_put
#define jo_get        json_object_object_get
#define jo_new        json_object_new_object
#define jo_array_add  json_object_array_add

/** Reads a JSON object from stream.
    Returns 0 on error. XXX: does not support unicode. */
JO json_read_stream(FILE*);

/** Skips one object from stream (without parsing it).
    Returns 0 on error. XXX: does not support unicode. */
int json_stream_skip(FILE*);

JO jo_new_double_array(const double *v, int n);
JO jo_new_int_array   (const int    *v, int n);

void jo_add_double       (JO, const char*name, double v);
void jo_add_int          (JO, const char*name, int    v);
void jo_add_double_array (JO, const char*name, const double *v, int n);
void jo_add_int_array    (JO, const char*name, const int    *v, int n);

/** Return 0 if there isn't a field called 'name' */
int jo_read_double       (JO, const char*name, double*p) ;
int jo_read_int          (JO, const char*name, int*p) ;
int jo_read_double_array (JO, const char*name, double *p, int n, double when_null);
int jo_read_int_array    (JO, const char*name, int    *p, int n, int    when_null);

/* returns 0 if NAN */
JO jo_double_or_null(double d);

JO find_object_with_name(JO root, const char*name);
JO json_tokener_parse_len(char *str, int len);
	
#endif