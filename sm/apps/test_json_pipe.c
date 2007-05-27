#include "../src/laser_data_json.h"


int main(int argc, char * argv[]) {
	
	JO jo; 
	
	while((jo = json_read_stream(stdin))) {
		printf(json_object_to_json_string(jo));
	}
	
	return 0;
}