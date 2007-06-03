#include <csm/csm.h>

int main(int argc, char * argv[]) {
	LDP ld;
	while((ld = ld_read_smart(stdin))) {
		
		JO jo = ld_to_json(ld);
		puts(json_object_to_json_string(jo));
		puts("\n");
		jo_free(jo);
		
		ld_free(ld);
	}
	
	return 0;
}
