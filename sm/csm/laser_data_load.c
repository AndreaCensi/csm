#include "csm_all.h"


/** Loads all laser data */

int ld_read_all(FILE*file, LDP **array, int*num) {
	*array = 0; *num = 0;
	int size = 10;
	LDP * ar = (LDP*) malloc(sizeof(LDP)*size);
	while(1) {
		LDP ld = ld_read_smart(file);
		if(!ld) break;
		ar[(*num)++] = ld;
		
		if(*num > size - 1) {
			size *= 2;
			if(! (ar = (LDP*) realloc(ar, sizeof(LDP)*size)) ) {
				sm_error("Cannot allocate (size=%d)\n", size);
				return 0;
			}
		}
	}

	*array = ar; 
	
	return feof(file);
}


