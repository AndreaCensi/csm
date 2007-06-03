#include <sys/param.h>
#include <stdarg.h>
#include <stdio.h>

#include "csm_all.h"

void sm_error(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_ERR, msg, ap);
	} else
#endif
	vfprintf(stderr, msg, ap);
}

void sm_debug(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_ERR, msg, ap);
	} else
#endif
	vfprintf(stderr, msg, ap);
}
