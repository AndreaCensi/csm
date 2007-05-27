#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include "bits.h"
#include "json_object.h"
#include "json_more_utils.h" 
#include "json_tokener.h"
#include "JSON_checker.h"

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
				mc_error("EOF while %d were read: \n\t'%.*s'. \n", count, count, buf);
				return 0;
			} 
			mc_error("Reading error: %s\n", strerror(errno));
			return 0;
		} else {
			if(count==0 && isspace(c)) continue;
			
			buf[count] = c;
			bufs[count] = c;
			count++;
			
			if(JSON_checker(bufs, count)) {
/*				sm_de("Found object.\n"); */
				JO jo = json_tokener_parse_len(buf, count);
				return jo;
			}
			
		}
	}
	
	mc_error("Object is bigger than MAX_SIZE = %d\n", MAX_SIZE);	
	return 0;
}


struct json_object* json_tokener_parse_len(char *str, int len) {
  struct json_tokener* tok;
  struct json_object* obj;

  tok = json_tokener_new();
  obj = json_tokener_parse_ex(tok, str, len);
  if(tok->err != json_tokener_success)
    obj = error_ptr(-tok->err);
  json_tokener_free(tok);
  return obj;
}



int json_read_double_array(JO s, const char*name, double*p, int n, double when_null) {
	JO jo = json_object_object_get(s, (char*)name);
	if(!jo) {
		mc_error("Field '%s' not found.", name);
		return 0;
	}
	
	if(!json_object_is_type(jo, json_type_array)) {
		mc_error("This is not an array: '%s'",json_object_to_json_string(jo));
		return 0;
	}
	int size = json_object_array_length(jo);
	if(size < n) {
		mc_error("I expected at least %d elements, got %d. \nArray: '%s'",
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
		mc_error("Field '%s' not found.", name);
		return 0;
	}
	if(!json_object_is_type(jo, json_type_array)) {
		mc_error("This is not an array: '%s'",json_object_to_json_string(jo));
		return 0;
	}
	int size = json_object_array_length(jo);
	if(size < n) {
		mc_error("I expected at least %d elements, got %d. \nArray: '%s'",
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


JO json_double_array(const double *v, int n) {
	JO array = json_object_new_array();
	int i; for(i=0;i<n;i++) {
		JO value = v[i] == v[i] ?  /* NAN is null */
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

