#ifndef H_OPTIONS
#define H_OPTIONS

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Utility functions to parse command line arguments.
 *
 *  See options_example.c.
 */

enum option_type { OPTION_STRING=0, OPTION_INT=1, OPTION_DOUBLE=2 };
 

struct option {
	/** Name of the option (or 0 if this is the last element). */
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
	 *      and you should free it yourself.
	 */
	void * value_pointer;


	/** If not NULL, it is set to 1 if the option is found. */
	int * set_pointer;
};


/** User-friendly interface */
struct option* options_allocate(int n);

void options_int    (struct option*, const char* name,  
	int *p,  int def_value, const char*desc);

void options_double (struct option*, const char* name,  
	double *p, double def_value, const char*desc);
	
void options_string (struct option*, const char* name, 
	const char** p,const char*def_balue,const char*desc);

/** Returns 0 on error */
int options_parse_args(struct option*ops, int argc, const char* argv[]);
/** Returns 0 on error */
int options_parse_file(struct option*ops, const char*pwd, const char*file);

void options_print_help(struct option*ops, FILE*where);

/** Internal use */

/** Finds an option in the array. Returns 0 if not found. */
struct option * options_find(struct option*ops, const char * name);

/** Returns true if the option needs an argument */
int options_requires_argument(struct option*o);

/** Flags the option as passed */
void options_set_passed(struct option*o);

/** Returns 0 on error */
int options_try_pair(struct option*ops, const char*name, const char*value);

/** Returns 0 on error. */
int options_set(struct option*op, const char*value);

int options_valid(struct option*op);

void options_dump(struct option * options, FILE*f, int write_desc);


#define OPTIONS_NAME_MAXSIZE 32
#define OPTIONS_VALUE_MAXSIZE 256

#ifdef __cplusplus
}
#endif



#endif
