#include <assert.h>
#include <string.h>
#include "../csm/csm_all.h"
#include <options/options.h>

#include <json-c/json.h>
#include <json-c/json_object_private.h>

void jo_write_as_matlab(JO jo, FILE*out);

void jo_write_as_matrix(JO jo, FILE*out);
void jo_write_as_column_vector(JO jo, FILE* out);

void jo_write_as_cell_array(JO jo, FILE* out);
int jo_is_numeric_matrix(JO jo);
int jo_is_numeric_array(JO jo);
void jo_write_as_matlab_object(JO jo, FILE*out);


const char * banner = 
"Converts JSON stream to Matlab file. \n"
"There are three usages: \n"
" 1) with only one parameter, \n"
"    $ json2matlab dir/mydata.json \n"
"    creates a Matlab function 'mydata' inside the file 'dir/mydata.m' \n"
" 2) with two parameters, \n"
"     $ json2matlab dir/mydata.json dir/out.m \n" 
"    creates a Matlab function 'out' inside the file 'dir/out.m'. \n"
" 3) otherwise, use the options switches. \n"
" \n"
" By default it creates a complete script of the kind:\n"
" \n"
"   function res = function_name()\n"
"      res = \n"
"		{ ...}\n"
" \n"
" If complete_script is set to 0, it just outputs the meat: \n"
" \n"
"		{ ...}\n"
" \n";


int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);

	const char * input_filename;
	const char * out_filename;
	const char * function;
	int complete_script;
	int debug;
	
	options_banner(banner);
	
	struct option* ops = options_allocate(8);
	options_string(ops, "in", &input_filename, "stdin", "input file (JSON)");
	options_string(ops, "out", &out_filename, "stdout", "output file (MATLAB)");
	options_string(ops, "function", &function, "", "Matlab function name (if empty, use basename of out)");
	options_int(ops, "complete_script", &complete_script, 1, "Write complete script 'function  res = ...'");
	options_int(ops, "debug", &debug, 1, "Shows debug information");
	
	if(argc == 2 && (argv[1][0] != '-')) { 
		/* one parameter */
		input_filename = argv[1]; int len = strlen(input_filename) + 4;
		char base[len], no_suffix[len], out[len];
		my_no_suffix(input_filename, no_suffix);
		sprintf(out, "%s.m", no_suffix);
		my_basename_no_suffix(input_filename, base);
		out_filename = my_strdup(out);
		function = my_strdup(base);
	} else if(argc == 3 && (argv[1][0] != '-') && (argv[2][0] != '-')) { 
		input_filename = argv[1]; 
		out_filename = argv[2];
	} else {
		/* FIXME help not shown */
		if(!options_parse_args(ops, argc, argv))
			return -1;
	}

	sm_debug_write(debug);

	if(!strcmp(function,"")) {
		int len = strlen(out_filename) + 4;
		char base[len];
		my_basename_no_suffix(out_filename, base);
		function = my_strdup(base);				
	}
	
	
	FILE * out = open_file_for_writing(out_filename);
	if(!out) return -2;

	FILE * in = open_file_for_reading(input_filename);
	if(!in) return -3;
	
	if(complete_script) {
		fprintf(out, "function res = %s\n", function);
		fprintf(out, " res = ... \n");
	}
	fprintf(out, " { ... \n\t");
	
	JO jo; 
	int i = 0;
	while((jo = json_read_stream(in))) {
		if(i>0) fprintf(out, ", ...\n\t");
		jo_write_as_matlab(jo, out);
		jo_free(jo);
		i++;
	}

	fprintf(out, "... \n }; \n");
	return 0;
}


void jo_write_as_matlab(JO jo, FILE*out) {
	if(!jo) { fprintf(out, "NaN"); return; } 
	
	switch(json_object_get_type(jo)) {
		case json_type_null: 
			fprintf(out, "NaN"); 
			return;
		
		case json_type_boolean:
			fprintf(out, json_object_get_boolean(jo) ? "true" : "false" );
			return;
		
		case json_type_int:
			fprintf(out, "%d", json_object_get_int(jo));		
			return;

		case json_type_double: 
			fprintf(out, "%lg", json_object_get_double(jo));		
			return;
		
		case json_type_object:
			jo_write_as_matlab_object(jo, out);
			return;
		
		case json_type_array:
			if(jo_is_numeric_matrix(jo))
			jo_write_as_matrix(jo, out);
			else
			if(jo_is_numeric_array(jo))
			jo_write_as_column_vector(jo, out);
			else 
			jo_write_as_cell_array(jo, out);		
		return;
		
		case json_type_string:
			fprintf(out, "'");
			const char* s = json_object_get_string(jo);
			while(*s) {
				if(*s==39) 
				fputc('"', out);
				else 
				fputc(*s, out);
				s++;
			}
				
			fprintf(out, "'");
			return;
	}
	
	
}



void jo_write_as_matlab_object(JO jo, FILE*out) {
	int i=0;
	struct json_object_iter iter;
	fprintf(out, "struct(");
	
	json_object_object_foreachC(jo, iter) {
		if(i) fprintf(out, ", ... \n\t ");
		fprintf(out, "'%s', ", iter.key);
		
		enum json_type t = json_object_get_type(iter.val);
		if( (t == json_type_array) && (!jo_is_numeric_matrix(iter.val)) && (!jo_is_numeric_array(iter.val))) {
			fprintf(out, "{");
			jo_write_as_matlab(iter.val, out);
			fprintf(out, "}");
		} else if(t == json_type_object) {
			fprintf(out, "{ ");
			jo_write_as_matlab(iter.val, out);
			fprintf(out, " }");			
		} else jo_write_as_matlab(iter.val, out);

		i++;
	}
	fprintf(out, ")");
}


int jo_is_numeric_matrix(JO jo) {
	/* null is not a matrix */
	if(!jo) return 0;
	if(json_object_get_type(jo) != json_type_array) return 0;
	int len = json_object_array_length(jo);
	int ncolumns = -1;
	for(int i=0;i<len;i++){
		JO row = json_object_array_get_idx(jo, i);
		if(!jo_is_numeric_array(row)) return 0;
		if(i==0) 
			ncolumns = json_object_array_length(row);
		else
			if(ncolumns !=  json_object_array_length(row))
			return 0;
	}
	if(ncolumns==0) return 0;
	return 1;
}

int jo_is_numeric_array(JO jo) {
	/* null is not an array */
	if(!jo) return 0;
	if(json_object_get_type(jo) != json_type_array) return 0;
	int len = json_object_array_length(jo);
	for(int i=0;i<len;i++){
		JO elem = json_object_array_get_idx(jo, i);

		/* I consider null elements as numeric values because they can be
		   converted into NaN */
		if(elem==0)
			continue;
			
		switch(json_object_get_type(elem)) {
			case json_type_null: 
			case json_type_boolean:
			case json_type_int:
			case json_type_double:
			continue;
			default:
			return 0;
		}
	}
	return 1;
}

void jo_write_as_matrix(JO jo, FILE*out) {
//		"[ " + map{|row| row.join(", ")}.join("; ... \n") +  "]"	
	assert(json_object_get_type(jo) == json_type_array);
	fprintf(out, "[");
	int len = json_object_array_length(jo);
	for(int i=0;i<len;i++){
		if(i>0) fprintf(out, ";  ");
		JO row = json_object_array_get_idx(jo, i);
		int n = json_object_array_length(row);
		for(int j=0;j<n;j++) {
			if(j>0) fprintf(out, ", ");
			jo_write_as_matlab(json_object_array_get_idx(row, j), out);
		}
	}
	fprintf(out, "]");		
}

void jo_write_as_column_vector(JO jo, FILE* out) {
//	"[  "+map{|x| x.to_matlab }.join("; ")+"]"
	assert(json_object_get_type(jo) == json_type_array);
	fprintf(out, "[");
	int len = json_object_array_length(jo);
	for(int i=0;i<len;i++){
		if(i>0) fprintf(out, "; ");
		JO elem = json_object_array_get_idx(jo, i);
		jo_write_as_matlab(elem, out);
	}
	fprintf(out, "]");
}

void jo_write_as_cell_array(JO jo, FILE* out) {
	assert(json_object_get_type(jo) == json_type_array);
	int len = json_object_array_length(jo);
	if(len==0) { 
		fprintf(out, "{}"); 
		return; 
	} else {	
		fprintf(out, "{ ");
		for(int i=0;i<len;i++){
			if(i>0) fprintf(out, ", ");
			JO elem = json_object_array_get_idx(jo, i);
			jo_write_as_matlab(elem, out);
		}
		fprintf(out, "}");
	}
//		"{ ... \n "+map{|x| x.to_matlab }.join(",  ... \n")+"}"
}
