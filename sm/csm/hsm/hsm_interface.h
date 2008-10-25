#ifndef H_HSM_INTERFACE
#define H_HSM_INTERFACE

#include <options/options.h>

#include "hsm.h"

struct sm_params;
struct sm_result;

/* Interface of HSM for CSM */
void sm_hsm(struct sm_params* params, struct sm_result* res);

/** Adds options related to HSM */
void hsm_add_options(struct option* ops, struct hsm_params*p);

#endif
