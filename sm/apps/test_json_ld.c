#include "../csm/csm_all.h"

int main() {
	JO jo; /* the monkey */
	LDP ld;
	
	while((jo = json_read_stream(stdin))) {
		if(!(ld = json_to_ld(jo))) {
			fprintf(stderr, "Could not transform to laser_data:\n\n");
			fprintf(stderr, "-----\n");
			fprintf(stderr, "%s", json_object_to_json_string(jo));
			fprintf(stderr, "-----\n");
			continue;
		}
		
		jo = ld_to_json(ld);
		printf("%s", json_object_to_json_string(jo));
		printf("\n");
	}
	
	return 0;
}
