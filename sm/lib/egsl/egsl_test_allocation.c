#include "egsl.h"
#include "egsl_macros.h"

int main() {
	egsl_push();

	val v1 = zeros(10,10);

	int i;
	for(i=0;i<1000000;i++) {
	       egsl_push();
	       val v2 = zeros(10,10);

	       add_to(v1, v2);
       
	       egsl_pop();
	}

	egsl_pop();
	egsl_print_stats();
	
	return 0;
}
