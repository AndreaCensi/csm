#ifndef H_JSON_JOURNAL
#define H_JSON_JOURNAL

#include <stdio.h>

#include "laser_data_json.h"

#define JJ 1

void jj_context_enter(const char*context_name);
void jj_context_exit();

void jj_loop_enter(const char*loop_name);
void jj_loop_iteration();
void jj_loop_exit();

void jj_add_int(const char*name, int);
void jj_add_double(const char*name, double);
void jj_add(const char*name, JO);

void jj_pop_and_flush(FILE*);

#endif