#include <options/options.h>
#include "../csm/csm_all.h"

void jo_write_plain(JO jo, FILE* out);

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	const char*input_filename;
	const char*output_filename;
	const char*field;
	int exit_on_error;
	
	struct option* ops = options_allocate(3);
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &output_filename, "stdout", "output file (JSON)");
	options_int(ops, "exit_on_error", &exit_on_error, 0, "if true, exit if object has no field");
	options_string(ops, "field", &field, "field_name", "field to extract from structure");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info("Extracts a field from JSON object."
			"\n\nOptions:\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * input_stream = open_file_for_reading(input_filename);
	FILE *output_stream = open_file_for_writing(output_filename);
	
	if(!input_stream || !output_stream) return -1;
	
	int n=0;
	while(1) {
		n++;
		
		JO jo = json_read_stream(input_stream);
		if(!jo) {
			if(feof(input_stream)) break;
			sm_error("Error while reading stream.\n");
			return -2;
		}
		
		JO jo_field = jo_get(jo, field);
		if(!jo_field) {
			if(exit_on_error) {
				sm_error("object #%d: field '%s' not found in structure.\n", n, field);
				return -1;
			} else {
				sm_error("object #%d: field '%s' not found in structure.\n", n, field);
				continue;
			}
		}
		
		jo_write_plain(jo_field, output_stream);
		fputs("\n", output_stream);
		jo_free(jo);
	};
	
	return 0;
}

void jo_write_plain(JO jo, FILE* out) {
	switch(json_object_get_type(jo)) {
		case json_type_boolean: {
			int v = (int) json_object_get_boolean(jo);
			fprintf(out, "%d", v);
			break;
		}
		case json_type_int: {
			int v = json_object_get_int(jo);
			fprintf(out, "%d", v);
			break;
		}
		case json_type_double: {
			double v = json_object_get_double(jo);
			fprintf(out, "%g", v);
			break;
		}
		case json_type_string: {
			const char * s = json_object_get_string(jo);
			fputs(s, out);
			break;
		}
		case json_type_object: {
			fputs("{object}", out);
			break;
		}
		case json_type_array: {
			int k, len = json_object_array_length(jo);
			for(k=0; k<len; k++) {
				JO v = json_object_array_get_idx(jo, k);
				jo_write_plain(v, out);
				if(k!=len-1) fputs(" ", out);
			}
			break;
		}

	default:
			sm_error("Unknown JSON type %d.\n", json_object_get_type(jo) );
	}
	
}

