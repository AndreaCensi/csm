#include <options/options.h>
#include "../csm/csm_all.h"

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
    int period; int phase;
	const char*input_filename;
	const char*output_filename;
	
	struct option* ops = options_allocate(3);
	options_int(ops, "period", &period, 1, "Period of objects to extract.");
	options_int(ops, "phase", &phase, 0, "Phase (=0 starts with the first object)");
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &output_filename, "stdout", "output file (JSON)");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s : decimates a JSON stream."
			"\n\nOptions:\n", argv[0]);
		options_print_help(ops, stderr);
		return -1;
	}
	
	if(period < 1) {
		sm_error("Period must be >= 1.\n");
		return 2;
	}
	
	FILE * input_stream = open_file_for_reading(input_filename);
	FILE *output_stream = open_file_for_writing(output_filename);
	
	if(!input_stream || !output_stream) return -1;
	
	
	int count = 0;
	while(1) { 
		JO jo = json_read_stream(input_stream);
		if(!jo) {
			if(feof(input_stream)) break;
			sm_error("Malformed JSON\n");
			return -1;
		}
		
		if( (count - phase) % period == 0) {
			const char * s = json_object_to_json_string(jo);
			fputs(s,output_stream); fputs("\n",output_stream);
		} 
		
		jo_free(jo);
		count++;
	}
	
	return 0;
}
