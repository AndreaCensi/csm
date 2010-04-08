#include <options/options.h>
#include "../csm/csm_all.h"

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	int nth;
	const char*input_filename;
	const char*output_filename;
	
	struct option* ops = options_allocate(3);
	options_int(ops, "nth", &nth, 0, "Index of object to extract.");
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &output_filename, "stdout", "output file (JSON)");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s : extracts n-th JSON object from stream."
			"\n\nOptions:\n", argv[0]);
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * input_stream = open_file_for_reading(input_filename);
	FILE *output_stream = open_file_for_writing(output_filename);
	
	if(!input_stream || !output_stream) return -1;
	
	int i; for(i=0;i<nth;i++) {
		if(!json_stream_skip(input_stream)) {
			sm_error("Could not skip %d-th object\n", i);
			return -2;
		}
	}
	
	JO jo = json_read_stream(input_stream);
	if(!jo) {
		fprintf(stderr, "Could not read %d-th object (after skipping %d)\n", 
			nth, i);
		return -2;
	}
	
	fputs(json_object_to_json_string(jo), output_stream);
	fputs("\n", output_stream);
	return 0;
}
