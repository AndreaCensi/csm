#include <options/options.h>
#include <string.h>
#include "../csm/csm_all.h"

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	const char*input_filename;
	const char*output_pattern_op;
	
	struct option* ops = options_allocate(3);
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &output_pattern_op, "./ld_split^02d.json", "output pattern; printf() pattern, but write '^' instead of '%'");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s : splits a JSON file into many files."
			"\n\nOptions:\n", argv[0]);
		options_print_help(ops, stderr);
		return -1;
	}
	
	
	/* Substitute "$" with "%" */
	
	char output_pattern[256]; strcpy(output_pattern, output_pattern_op);
	char *f = output_pattern;
	while(*f) {
		if(*f=='^') *f='%';
		f++;
	}

	fputs(output_pattern, stderr);
	
	FILE * input_stream = open_file_for_reading(input_filename);

	int count = 0;
	
	JO jo;
	while( (jo = json_read_stream(input_stream)) ) {
		char filename[1000];
		sprintf(filename, output_pattern, count);
		if(!count) {
			
		}
		
		sm_debug("Writing to file (%s) %s\n", output_pattern, filename);
		FILE * f = open_file_for_writing(filename);
		if(!f) return -1;
		fputs(json_object_to_json_string(jo), f);
		jo_free(jo);
		fclose(f);
		
		count++;
	}
	
	
	return 0;
}
