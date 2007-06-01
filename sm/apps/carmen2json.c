#include "../src/laser_data_json.h"

int main(int argc, char * argv[]) {
	LDP ld;
	while((ld = ld_read_smart(stdin))) {
		
		JO jo = ld_to_json(ld);
		printf(json_object_to_json_string(jo));
		printf("\n");
		jo_free(jo);
		
		ld_free(ld);
	}
	
	return 0;
}