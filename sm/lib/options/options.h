/* $Id: options.h 764 2005-04-26 08:45:06Z andrea $ */
#ifndef H_OPTIONS
#define H_OPTIONS

#include <stdio.h>

/**
 *  Utility functions to parse command line arguments.
 *
 *  See options_example.c.
 */

 enum option_type { OPTION_STRING, OPTION_INT, OPTION_DOUBLE };
 
struct option {
	/** Name of the option. */
	const char * name;
	const char * desc;

	/** Value type (if any, otherwise ignored). */
	enum option_type type;
	
	/** Pointer to store param value. If value_pointer ==NULL then
	 *  the option has no parameters. Ex: in "--port 2000", "--port"
	 *  is the name and "2000" is the value. value_pointer is interpreted
	 *  according to the value of "type".
	 *   type=   INT:	value_pointer is a "int *"
	 *   type=STRING:	value_pointer is a "char **"
	 *   type=DOUBLE:	value_pointer is a "double *"
	 *      A new string is allocated using malloc():
	 *          *(value_pointer) = malloc( ... )
	 */
	void * value_pointer;


	/** If not NULL, it is set to 1 if the option is found. */
	int * set_pointer;
};


/* 0 on error */
int auto_option(int argc, const char* argv[], int noptions, struct option * options);

void print_help(FILE*where, int noptions, struct option * options);

#endif
