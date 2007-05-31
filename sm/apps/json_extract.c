#include <libgen.h>
#include "../src/laser_data_json.h"
#include <options/options.h>

int main(int argc, const char * argv[]) {
	
	int nth;
	
	struct option* ops = options_allocate(3);
	options_int(ops, "nth", &nth, 0, "Index of object to extract.");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s : extracts n-th JSON object from stream."
			"\n\nOptions:\n", basename(argv[0]));
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * stream = stdin;
	
	int i; for(i=0;i<nth;i++) {
		if(!json_stream_skip(stream)) {
			fprintf(stderr, "Could not skip %d-th object\n", i);
			return -2;
		}
	}
	
	JO jo = json_read_stream(stream);
	if(!jo) {
		fprintf(stderr, "Could not read %d-th object (after skipping %d)\n", 
			nth, i);
		return -2;
	}
	
	printf(json_object_to_json_string(jo));
	printf("\n");
	return 0;
}