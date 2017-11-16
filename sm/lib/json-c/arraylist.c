/*
 * $Id: arraylist.c,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#endif /* STDC_HEADERS */

#if HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include "bits.h"
#include "arraylist.h"

struct array_list*
array_list_new(array_list_free_fn *free_fn)
{
  struct array_list *this;

  if(!(this = calloc(1, sizeof(struct array_list)))) return NULL;
  this->size = ARRAY_LIST_DEFAULT_SIZE;
  this->length = 0;
  this->free_fn = free_fn;
  if(!(this->array = calloc(sizeof(void*), (size_t) this->size))) {
    free(this);
    return NULL;
  }
  return this;
}

extern void
array_list_free(struct array_list *this)
{
  int i;
  for(i = 0; i < this->length; i++)
    if(this->array[i]) this->free_fn(this->array[i]);
  free(this->array);
  free(this);
}

void*
array_list_get_idx(struct array_list *this, int i)
{
  if(i >= this->length) return NULL;
  return this->array[i];
}

static int array_list_expand_internal(struct array_list *this, int max)
{
  void *t;
  int new_size;

  if(max < this->size) return 0;
  new_size = MAX(this->size << 1, max);
  if(!(t = realloc(this->array, new_size*sizeof(void*)))) return -1;
  this->array = t;
  (void)memset(this->array + this->size, 0, (new_size-this->size)*sizeof(void*));
  this->size = new_size;
  return 0;
}

int
array_list_put_idx(struct array_list *this, int idx, void *data)
{
  if(array_list_expand_internal(this, idx)) return -1;
  if(this->array[idx]) this->free_fn(this->array[idx]);
  this->array[idx] = data;
  if(this->length <= idx) this->length = idx + 1;
  return 0;
}

int
array_list_add(struct array_list *this, void *data)
{
  return array_list_put_idx(this, this->length, data);
}

int
array_list_length(struct array_list *this)
{
  return this->length;
}
