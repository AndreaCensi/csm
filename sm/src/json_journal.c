#include <assert.h>
#include "json_journal.h"

#define MAX_STACK 1000

JO jj_stack[MAX_STACK];
int jj_stack_index = -1;

JO jj_stack_top() {
	assert(jj_stack_index>=0);
	return jj_stack[jj_stack_index];
}

void jj_stack_push(JO jo) {
	assert(jj_stack_index<MAX_STACK);
	jj_stack[++jj_stack_index] = jo;
}

void jj_stack_pop() {
	assert(jj_stack_index>=0);
	if(jj_stack_index == 0) {
		fprintf(stderr, "\n-------\n", json_object_to_json_string(jj_stack_top()));
		jo_free(jj_stack_top());
	}
	jj_stack_index--;
}

void jj_context_enter(const char*context_name) {
	JO jo = json_object_new_object();
	if(jj_stack_index>=0)
	jo_add(jj_stack_top(), (char*)context_name, jo);
	
	jj_stack_push(jo);
}

void jj_must_be_hash() {
	assert(json_object_is_type(jj_stack_top(), json_type_object));	
}

void jj_must_be_array() {
	assert(json_object_is_type(jj_stack_top(), json_type_array));	
}

void jj_context_exit() {
	jj_must_be_hash();
	jj_stack_pop();
}

void jj_loop_enter(const char*loop_name) {
	jj_must_be_hash();
	JO jo = json_object_new_array();
	jo_add(jj_stack_top(), loop_name, jo);
	jj_stack_push(jo);
}

void jj_loop_iteration() {
	JO this_iteration = json_object_new_object();
	if(!json_object_is_type(jj_stack_top(), json_type_array)) {
		jj_stack_pop();
		jj_must_be_array();
	}
	json_object_array_add(jj_stack_top(), this_iteration);
	jj_stack_push(this_iteration);
}

void jj_loop_exit() {
	if(!json_object_is_type(jj_stack_top(), json_type_array))
		jj_stack_pop();
		
	jj_must_be_array();
	jj_stack_pop();
}

void jj_add_int(const char*name, int v) {
	jj_must_be_hash();
	jo_add(jj_stack_top(), name, jo_new_int(v));
}

void jj_add_double(const char*name, double v) {
	jj_must_be_hash();
	jo_add(jj_stack_top(), name, jo_double_or_null(v));
}

void jj_add(const char*name, JO jo) {
	jj_must_be_hash();
	jo_add(jj_stack_top(), name, jo);
}

void jj_pop_and_flush(FILE*f) {
	jj_must_be_hash();
	
}
