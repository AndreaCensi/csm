#include <libgen.h>
#include "../src/laser_data_json.h"
#include <options/options.h>

int main(int argc, const char * argv[]) {
	
	int n;
	
	struct option* ops = options_allocate(3);
	options_int(ops, "n", &n, 1, "Number of copies");
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s : reads a JSON stream and copies it multiplied by n."
			"\n\nOptions:\n", basename(argv[0]));
		options_print_help(ops, stderr);
		return -1;
	}
	
	JO jo; 
	
	while((jo = json_read_stream(stdin))) {
		int i; for(i=0;i<n;i++) {
			printf(json_object_to_json_string(jo));
			printf("\n");
		}
	}
	
	return 0;
}