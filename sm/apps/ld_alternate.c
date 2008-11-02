#include "../csm/csm_all.h"

int main(int argc, const char ** argv) {
	sm_set_program_name(argv[0]);

	int every = 5;
	int count = 0;
	LDP ld;
	while( (ld = ld_read_smart(stdin))) {
		count++;
		
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			continue;
		}

		int i;
		for(i=0;i<ld->nrays;i++) {
			if( (i % every) != 0 ) {
				ld->valid[i] = 0;
				ld->readings[i] = NAN;
			}
		}
		
		ld_write_as_json(ld, stdout);
		ld_free(ld);

		count++;
	}
	
	return 0;
}
