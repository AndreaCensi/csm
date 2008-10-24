#include <string.h>
#include <pgm.h>
#include <options/options.h>

#include <csm/csm_all.h>

#include "hsm.h"
#include "hsm_interface.h"

const char * banner = 
	"Reads an image, computes HT, de-computes it \n\n";

struct {
	const char * file_input1;
	const char * file_input2;
	
	const char * prefix;
	int debug;
	
	struct hsm_params hsmp;
	
} p;

int main(int argc, const char**argv) {
	pgm_init(&argc, argv);
	options_banner(banner);
	
	
	struct option* ops = options_allocate(20);
	options_string(ops, "in1", &p.file_input1, "stdin", "Input file 1");
	options_string(ops, "in2", &p.file_input2, "", "Input file 2");
	options_string(ops, "out", &p.prefix, "test00", "Output file prefix ");
	
		hsm_add_options(ops, &p.hsmp);
	options_int(ops, "debug", &p.debug, 0, "Shows debug information");
	
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}
	
	sm_debug_write(p.debug);


	

	hsm_buffer b1 = create_ht_for_image(&(p.hsmp), in1, 0);  if(!b1) return -3;
	hsm_compute_spectrum(b1);


	if(!strcmp(p.file_input2,"")) {
		p.file_input2 = p.file_input1;
		p.hsmp.debug_true_x_valid = 1;
		p.hsmp.debug_true_x[0] = 0;
		p.hsmp.debug_true_x[1] = 50;
		p.hsmp.debug_true_x[2] = deg2rad(40.0);
	} else {
		p.hsmp.debug_true_x_valid = 0;
	}

	FILE * in2 = open_file_for_reading(p.file_input2);       if(!in2 ) return -2;
	sm_debug("Computing HT for image %s.\n", p.file_input2);
	double *base = p.hsmp.debug_true_x_valid ? p.hsmp.debug_true_x : 0;
	hsm_buffer b2 = create_ht_for_image(&(p.hsmp), in2, base);     if(!b2) return -3;	
	hsm_compute_spectrum(b2);
	
	
	
	sm_debug("Doing scan-matching..\n"); 
	p.hsmp.max_translation = max(b1->rho_max, b2->rho_max);
	
	hsm_match(&(p.hsmp),b1,b2);

	

}


