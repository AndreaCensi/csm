#include <errno.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "logging.h"
//#include <strerror.h>

#include <JSON_checker.h>
#include "laser_data_json.h"

JO json_double_array(const double *v, int n) {
	JO array = json_object_new_array();
	int i; for(i=0;i<n;i++) {
		JO value = v[i] == v[i] ? 
			json_object_new_double(v[i]) : json_tokener_parse("null");
		json_object_array_add(array, value);
	}
	return array;
}

JO json_int_array(const int *v, int n) {
	JO array = json_object_new_array();
	int i; for(i=0;i<n;i++) {
		json_object_array_add(array, json_object_new_int(v[i]));
	}
	return array;
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
	return jo;
/*	int *up_bigger, *up_smaller, *down_bigger, *down_smaller;

	gsl_vector**p;
	
	struct correspondence* corr;
*/
}

int json_read_int(JO jo, const char*name, int*p) {
	JO v = json_object_object_get(jo, (char*)name);
	
	if(!v) {
		sm_error("Field '%s' not found.", name);
		return 0;
	}
	
	if(!json_object_is_type(v, json_type_int)) {
		sm_error("I was looking for a int, instead got '%s'.",
		         json_object_to_json_string(v));
		return 0;
	}
	
	*p = json_object_get_int(v);
	return 1;
}

double convert_to_double(JO jo) {
	if(json_object_is_type(jo, json_type_double)) 
		return json_object_get_double(jo);
	else 
		return NAN;
}

int json_read_double(JO jo, const char*name, double*p) {
	JO v = json_object_object_get(jo, (char*)name);
	
	if(!v) {
		sm_error("Field '%s' not found.", name);
		return 0;
	}
	
	*p = convert_to_double(v);
	return 1;
}

int json_read_double_array(JO s, const char*name, double*p, int n, double when_null) {
	JO jo = json_object_object_get(s, (char*)name);
	if(!jo) {
		sm_error("Field '%s' not found.", name);
		return 0;
	}
	
	if(!json_object_is_type(jo, json_type_array)) {
		sm_error("This is not an array: '%s'",json_object_to_json_string(jo));
		return 0;
	}
	int size = json_object_array_length(jo);
	if(size < n) {
		sm_error("I expected at least %d elements, got %d. \nArray: '%s'",
			n, size, json_object_to_json_string(jo));
		return 0;
	}
	int i; for(i=0;i<n;i++) {
		JO v = json_object_array_get_idx(jo, i);
		if(!v || !json_object_is_type(v, json_type_double))
			p[i] = when_null;
		else
			p[i] = json_object_get_double(v);
	}
	return 0;
}

int json_read_int_array(JO s, const char*name, int*p, int n, int when_null) {
	JO jo = json_object_object_get(s, (char*)name);
	if(!jo) {
		sm_error("Field '%s' not found.", name);
		return 0;
	}
	if(!json_object_is_type(jo, json_type_array)) {
		sm_error("This is not an array: '%s'",json_object_to_json_string(jo));
		return 0;
	}
	int size = json_object_array_length(jo);
	if(size < n) {
		sm_error("I expected at least %d elements, got %d. \nArray: '%s'",
			n, size, json_object_to_json_string(jo));
		return 0;
	}
	int i; for(i=0;i<n;i++) {
		JO v = json_object_array_get_idx(jo, i);
		if(!v || !json_object_is_type(v, json_type_int))
			p[i] = when_null;
		else
			p[i] = json_object_get_int(v);
	}
	return 0;
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
	
	
	return ld;
}

struct json_object* json_tokener_parse_len(char *str, int len)
{
  struct json_tokener* tok;
  struct json_object* obj;

  tok = json_tokener_new();
  obj = json_tokener_parse_ex(tok, str, len);
  if(tok->err != json_tokener_success)
    obj = error_ptr(-tok->err);
  json_tokener_free(tok);
  return obj;
}


JO json_read_stream(FILE*f) {
	#define MAX_SIZE 100000
	char buf[MAX_SIZE+1];
	unsigned short bufs[MAX_SIZE+1];
	
	int count = 0;
	
	while(count < MAX_SIZE) {
		char c;
		if(1 != fread(&c,1,1,f)) {
			if(feof(f)) {
				if(count==0) return 0;
				sm_error("EOF while %d were read: \n\t'%.*s'. \n", count, count, buf);
				return 0;
			} 
			sm_error("Reading error: %s\n", strerror(errno));
			return 0;
		} else {
			if(count==0 && isspace(c)) continue;
			
			buf[count] = c;
			bufs[count] = c;
			count++;
			
			if(JSON_checker(bufs, count)) {
//				sm_de("Found object.\n");
				JO jo = json_tokener_parse_len(buf, count);
				return jo;
			}
			
		}
	}
	
	sm_error("Object is bigger than MAX_SIZE = %d\n", MAX_SIZE);	
	return 0;
}





