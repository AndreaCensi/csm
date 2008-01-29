#include <string.h>
#include <errno.h>

#include "csm_all.h"

/** Wraps around fopen and provides error message. */
FILE * open_file(const char *filename, const char*mode);


FILE * open_file(const char *filename, const char*mode) {
	FILE*file = fopen(filename, mode);
	if(file==NULL) {
		sm_error("Could not open file '%s': %s.\n", filename, strerror(errno)); 
		return 0;
	}
	return file;
}

FILE * open_file_for_reading(const char*filename) {
	if(!strcmp(filename, "-"    )) return stdin;
	if(!strcmp(filename, "stdin")) return stdin;
	return open_file(filename, "r");
}

FILE * open_file_for_writing(const char*filename) {
	if(!strcmp(filename, "-"     )) return stdout;
	if(!strcmp(filename, "stdout")) return stdout;
	if(!strcmp(filename, "stderr")) return stderr;
	return open_file(filename, "w");
}

