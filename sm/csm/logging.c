#include <sys/param.h>
#include <stdarg.h>
#include <stdio.h>

#include "csm_all.h"

const char * sm_program_name = 0;

#define CSM_DEBUG 

void sm_error(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	if(sm_program_name) {
		fputs(sm_program_name, stderr);
		fputs(": ", stderr);
	}
	vfprintf(stderr, msg, ap);
}

void sm_debug(const char *msg, ...)
{
	#ifdef CSM_DEBUG
	va_list ap;
	va_start(ap, msg);
	if(sm_program_name) {
		fputs(sm_program_name, stderr);
		fputs(": ", stderr);
	}
	vfprintf(stderr, msg, ap);
	#else
	msg = 0;
	#endif
}
