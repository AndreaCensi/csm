#include "../src/laser_data_json.h"

int main(int argc, char * argv[]) {
	
	LDP ld = ld_alloc_new(50);
	JO jo = ld_to_json(ld);
	
	printf(json_object_to_json_string(jo));
	
	return 0;
}