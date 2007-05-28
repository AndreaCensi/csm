#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

/* Cavillo (non impatta la portabilità. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include "options.h"

/** User-friendly interface */
struct option* options_allocate(size_t n) {
	struct option* ops = malloc(sizeof(struct option)*n);
	size_t i; for(i=0;i<n;i++) {
		ops[i].name = 0;
		ops[i].type = 0xbeef;
		ops[i].desc = 0;
		ops[i].value_pointer = 0;
		ops[i].set_pointer = 0;
	}
	return ops;
}

/* XXX farlo meglio */
struct option* options_next_empty(struct option*ops) {
	int i; for(i=0;;i++) {
		if(ops[i].name == 0)
		 return ops+i;
	}
}

char * strdup_(const char *s);

void options_int(struct option*ops, const char* name, int *p, int def_value, const char*desc) {
	struct option* o =  options_next_empty(ops);
	o->name = strdup_(name);
	o->value_pointer = p;
	o->set_pointer = 0;
	o->desc = strdup_(desc);
	o->type = OPTION_INT;
	*p = def_value;
}

void options_double (struct option*ops, const char* name, double *p, double def_value, const char*desc){
	struct option* o =  options_next_empty(ops);
	o->name = strdup_(name);
	o->value_pointer = p;
	o->set_pointer = 0;
	o->desc = strdup_(desc);
	o->type = OPTION_DOUBLE;
	*p = def_value;
}

void options_string (struct option*ops, const char* name, const char** p,const char*def_value,const char*desc){
	struct option* o =  options_next_empty(ops);
	o->name = strdup_(name);
	o->value_pointer = p;
	o->set_pointer = 0;
	o->desc = strdup_(desc);
	o->type = OPTION_STRING;
	*p = def_value;
}
