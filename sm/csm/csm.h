#ifndef H_CSM_H
#define H_CSM_H

/* Some preprocessor magic for calling this library from C++ */

#ifdef __cplusplus
	namespace CSM {}
	extern "C" {
#endif

#include "laser_data.h"
#include "laser_data_drawing.h"
#include "laser_data_json.h"
#include "algos.h"
#include "utils.h"

#ifdef __cplusplus
	}
#endif

#endif
