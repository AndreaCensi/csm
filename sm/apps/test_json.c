#include "../csm/csm.h"

int main() {
	
	LDP ld = ld_alloc_new(50);
	JO jo = ld_to_json(ld);
	
	printf("%s", json_object_to_json_string(jo));
	
	return 0;
}
