#ifndef H_CSM_H
#define H_CSM_H

#define INLINE static inline
#define INLINE_DECL static inline

#ifdef __cplusplus
	#define restrict /* nothing */
	namespace CSM {
	extern "C" {
#endif

#include "laser_data.h"
#include "laser_data_json.h"
#include "algos.h"
#include "utils.h"


#ifdef __cplusplus
	}}
#endif

#endif
