#ifndef H_MAP_PAN
#define H_MAP_PAN

#include <csm/csm_all.h>

using namespace CSM;

struct merge_params {
	
};


LDP pan_new(int nrays);

void pan_merge(LDP pan, LDP ld);



#endif
