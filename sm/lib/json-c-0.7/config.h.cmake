/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#define HAVE_DOPRNT ${HAVE_DOPRNT}

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H ${HAVE_FCNTL_H}

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H ${HAVE_INTTYPES_H}

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H ${HAVE_LIMITS_H}

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC ${HAVE_MALLOC}

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H ${HAVE_MEMORY_H}

/* Define to 1 if you have the `open' function. */
#define HAVE_OPEN ${HAVE_OPEN}

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#define HAVE_REALLOC ${HAVE_REALLOC}

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H ${HAVE_STDARG_H}

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H ${HAVE_STDINT_H}

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H ${HAVE_STDLIB_H}

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR ${HAVE_STRERROR}

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H ${HAVE_STRINGS_H}

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H ${HAVE_STRING_H}

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP ${HAVE_STRNCASECMP}

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP ${HAVE_STRNDUP}

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H ${HAVE_SYSLOG_H}

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H ${HAVE_SYS_PARAM_H}

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H ${HAVE_SYS_STAT_H}

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H ${HAVE_SYS_TYPES_H}

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H ${HAVE_UNISTD_H}

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF ${HAVE_VASPRINTF}
 
/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF ${HAVE_VPRINTF}

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF ${HAVE_VSNPRINTF}

/* Define to 1 if you have the `vsyslog' function. */
#define HAVE_VSYSLOG ${HAVE_VSYSLOG}


/* Define to rpl_malloc if the replacement function should be used. */
#if !HAVE_MALLOC
#define malloc rpl_malloc
#endif

/* Define to rpl_realloc if the replacement function should be used. */
#if !HAVE_REALLOC
#define realloc rpl_realloc
#endif






/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t
