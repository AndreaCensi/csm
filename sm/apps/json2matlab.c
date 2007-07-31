#include <csm/csm.h>
#include <options/options.h>

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);

	const char * input_filename;
	const char * out_filename;
	const char * function;
	
	struct option* ops = options_allocate(3);
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &out_filename, "stdout", "output file");
	options_string(ops, "function", &function, "myfun", "Matlab function name");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info("Converts JSON stream to Matlab file"
			"\n\nOptions:\n", (char*)argv[0] );
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * out = open_file_for_writing(out_filename);
	if(!out) return -2;

	FILE * in = open_file_for_reading(input_filename);
	if(!in) return -3;
	
	fprintf(out, "function res = %s\n", function);
	fprintf(out, " res = ... \n", function);
	fprintf(out, " [ ... \n", function);
	
	JO jo; 
	while((jo = json_read_stream(in))) {
		jo_to_matlab(jo, out);
		
		jo_free(jo);
	}
	
	fprintf(out, "; \n");
	return 0;
}
