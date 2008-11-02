#include "../csm/csm_all.h"

int main(int argc, char * argv[]) {
	sm_set_program_name(argv[0]);

	LDP ld; int count=0, errors=0; 
	while((ld = ld_read_smart(stdin))) {
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			errors++;
			continue;
		}

		ld_write_as_carmen(ld, stdout);
		
		ld_free(ld);
		count++;
	}
	
	return errors;
}
