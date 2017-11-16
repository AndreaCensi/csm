/*
 * $Id: printbuf.c,v 1.5 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_STDARG_H
# include <stdarg.h>
#else /* !HAVE_STDARG_H */
# error Not enough var arg support!
#endif /* HAVE_STDARG_H */

#include "bits.h"
#include "debug.h"
#include "printbuf.h"

struct printbuf* printbuf_new()
{
  struct printbuf *p;

  if(!(p = calloc(1, sizeof(struct printbuf)))) return NULL;
  p->size = 32;
  p->bpos = 0;
  if(!(p->buf = malloc( (size_t) p->size))) {
    free(p);
    return NULL;
  }
  return p;
}


int printbuf_memappend(struct printbuf *p, const char *buf, int size)
{
  char *t;
  if(p->size - p->bpos <= size) {
    int new_size = MAX(p->size * 2, p->bpos + size + 8);
#ifdef PRINTBUF_DEBUG
    mc_debug("printbuf_memappend: realloc "
	     "bpos=%d wrsize=%d old_size=%d new_size=%d\n",
	     p->bpos, size, p->size, new_size);
#endif /* PRINTBUF_DEBUG */
    if(!(t = realloc(p->buf,  (size_t) new_size))) return -1;
    p->size = new_size;
    p->buf = t;
  }
  memcpy(p->buf + p->bpos, buf,  (size_t) size);
  p->bpos += size;
  p->buf[p->bpos]= '\0';
  return size;
}

#if !HAVE_VSNPRINTF && defined(WIN32)
# define vsnprintf _vsnprintf
#elif !HAVE_VSNPRINTF /* !HAVE_VSNPRINTF */
# error Need vsnprintf!
#endif /* !HAVE_VSNPRINTF && defined(WIN32) */

#if !HAVE_VASPRINTF
/* CAW: compliant version of vasprintf */
static int vasprintf(char **buf, const char *fmt, va_list ap)
{
#ifndef WIN32
	static char _T_emptybuffer = '\0';
#endif /* !defined(WIN32) */
	int chars;
	char *b;

	if(!buf) { return -1; }

#ifdef WIN32
	chars = _vscprintf(fmt, ap)+1;
#else /* !defined(WIN32) */
	/* CAW: RAWR! We have to hope to god here that vsnprintf doesn't overwrite
	   our buffer like on some 64bit sun systems.... but hey, its time to move on */
	chars = vsnprintf(&_T_emptybuffer, 0, fmt, ap)+1;
	if(chars < 0) { chars *= -1; } /* CAW: old glibc versions have this problem */
#endif /* defined(WIN32) */

	b = (char*)malloc(sizeof(char)*chars);
	if(!b) { return -1; }

	if((chars = vsprintf(b, fmt, ap)) < 0)
	{
		free(b);
	} else {
		*buf = b;
	}

	return chars;
}
#endif /* !HAVE_VASPRINTF */

int sprintbuf(struct printbuf *p, const char *msg, ...)
{
  va_list ap;
  char *t;
  int size;
  char buf[128];

  /* user stack buffer first */
  va_start(ap, msg);
  size = vsnprintf(buf, 128, msg, ap);
  va_end(ap);
  /* if string is greater than stack buffer, then use dynamic string
     with vasprintf.  Note: some implementation of vsnprintf return -1
     if output is truncated whereas some return the number of bytes that
     would have been writen - this code handles both cases. */
  if(size == -1 || size > 127) {
    int ret;
    va_start(ap, msg);
    if((size = vasprintf(&t, msg, ap)) == -1) return -1;
    va_end(ap);
    ret = printbuf_memappend(p, t, size);
    free(t);
    return ret;
  } else {
    return printbuf_memappend(p, buf, size);
  }
}

void printbuf_reset(struct printbuf *p)
{
  p->buf[0] = '\0';
  p->bpos = 0;
}

void printbuf_free(struct printbuf *p)
{
  if(p) {
    free(p->buf);
    free(p);
  }
}
