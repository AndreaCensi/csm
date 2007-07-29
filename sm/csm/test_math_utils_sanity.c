#include "csm_all.h"
#include <time.h>

#include "math_utils.h"

int main() {

	int errors = 0;
	double should_be_nan[2] = { 0.0 / 0.0, GSL_NAN };
	
	int i;
	for(i=0;i<2;i++) {
		if(!isnan(should_be_nan[i])) {
			printf("#%d: isnan(%f) failed \n", i, should_be_nan[i]);
			errors++;
		}
		if(!is_nan(should_be_nan[i])) {
			printf("#%d: is_nan(%f) failed \n", i, should_be_nan[i]);
			errors++;
		}
	}
	
	return errors;
}
