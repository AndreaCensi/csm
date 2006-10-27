/* $Id: options.cpp 764 2005-04-26 08:45:06Z andrea $ */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

/* Cavillo (non impatta la portabilità. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include "options.h"

/** Our version of strdup. */
char * strdup_(const char *s) {
	size_t len = strlen(s) + 1; /* null byte */
	//char * t = (char*) malloc(len);
	char * t = (char*)malloc(len);
	memcpy(t,s,len);
	return t;
}

/** Return 1 if ok. */
int get_int(int*p, const char*s) {
	int value;
	errno = 0;
	value = strtol(s, (char **)NULL, 10);
	if(0 == errno) {
		*p = value;
		return 1;
	} else return 0;
}

/** Return 1 if ok. */
int get_double(double*p, const char*s) {
	char *endptr;
	*p = strtod(s, &endptr);
	return endptr != s;
}

int auto_option(int argc, const char* argv[], int noptions, struct option*op) {
	int i;
	for (i=1; i<argc; i++) {
		int j, found = 0;
		for (j=0;j<noptions && !found;j++) {
			const char * name;
			int * set_pointer;
			int requires_argument;

			name = op[j].name;
			if (strcmp(argv[i],name))
				continue;

			found = 1;

			set_pointer = op[j].set_pointer;
			if(NULL!=set_pointer)
				*set_pointer = 1;

			
			requires_argument = NULL != op[j].value_pointer;

			if(requires_argument) {
				if(i>=argc-1) {
					fprintf(stderr, "Argument %s needs value.\n", op[j].name);
					return 0;
				}

				switch(op[j].type) {
					case(OPTION_INT): {
						int * value_pointer = (int*) op[j].value_pointer;
						int ok = get_int(value_pointer, argv[i+1]);
						if(!ok) {
							fprintf(stderr, "Could not parse int: %s = %s.\n", 
								op[j].name, argv[i+1]);
							return 0;
						}
						break;
					}

					case(OPTION_STRING): {
						char** value_pointer = (char**) op[j].value_pointer;
						*value_pointer = (char*) strdup_(argv[i+1]);
/*						fprintf(stderr, 
							"String %s, value_pointer=%p void pointer=%p *value_pointer=%p result=%s\n"
						 ,argv[i+1], value_pointer, o->op[j].value_pointer, *value_pointer,
						 *value_pointer);*/
						 break;
					}
					
					case(OPTION_DOUBLE): {
						double * value_pointer = (double*) op[j].value_pointer;
						int ok = get_double(value_pointer, argv[i+1]);
						if(!ok) {
						fprintf(stderr, "Could not parse double: %s = %s.\n", 
								op[j].name, argv[i+1]);
							return 0;
						}
						break;
					}
				} /* switch */
				i++;
			} else { /* doesn't require argument */

			}
		} /* for */

		if(!found) {
			fprintf(stderr, "Argument '%s' not found.\n", argv[i]);
			return 0;
		}
	} /* for */
	
	return 1;
}


void print_help(FILE*f, int noptions, struct option * options) {
	int j;
	for (j=0;j<noptions;j++) {
		fprintf(f, "%s\t\t\t%s ", options[j].name, options[j].desc);
			
		if(options[j].value_pointer)
		switch(options[j].type) {
			case(OPTION_INT): {
				int * value_pointer = (int*) options[j].value_pointer;
				fprintf(f, "default: '%d'", *value_pointer);
				break;
			}

			case(OPTION_STRING): {
				char** value_pointer = (char**) options[j].value_pointer;
				fprintf(f, "default: '%s'", *value_pointer);
				break;
			}
			
			case(OPTION_DOUBLE): {
				double * value_pointer = (double*) options[j].value_pointer;
				fprintf(f, "default: '%g'", *value_pointer);
				break;
			}
		} /* switch */
		
		fprintf(f,"\n");

	}
}
