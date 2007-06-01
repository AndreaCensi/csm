#include "options.h"

struct params {
	int a_int;
	double a_double;
	const char * file;
};

int main(int argc, const char*argv[]) {
	struct params p;
	
	struct option* ops = options_allocate(3);
	options_int    (ops, "i", &p.a_int,  42, "An int parameter");
	options_double (ops, "d", &p.a_double,  0.42, "A double parameter");
	options_string (ops, "s", &p.file ,  "Hello", "A file parameter");
	
	if(!options_parse_args(ops, argc, argv)) {
		printf("A simple program.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	printf("i: %d \n", p.a_int);
	printf("d: %g \n", p.a_double);
	printf("s: %s \n", p.file);
	
	return 0;
}
