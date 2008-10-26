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

hsm_buffer create_ht_for_image(struct hsm_params*p, FILE*in, const double base[3]);
void write_ht_on_image(hsm_buffer b, FILE*out);
void write_function_on_image(int n, const double*f, int rows, FILE*out);


int main(int argc, const char**argv) {
	pgm_init(&argc, argv);
	options_banner(banner);
	
	
	struct option* ops = options_allocate(20);
	options_string(ops, "in1", &p.file_input1, "stdin", "Input file 1");
	options_string(ops, "in2", &p.file_input2, "", "Input file 2");
	options_string(ops, "out", &p.prefix, "test00", "Output file prefix ");
	
	hsm_add_options(ops, &p.hsmp);
	p.hsmp.linear_cell_size = 1; /* 1 pixel */
		
	options_int(ops, "debug", &p.debug, 0, "Shows debug information");
	
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}
	
	sm_debug_write(p.debug);


	FILE * in1 = open_file_for_reading(p.file_input1);   if(!in1) return -2;

	sm_debug("Computing HT for image %s.\n", p.file_input1);

	hsm_buffer b1 = create_ht_for_image(&(p.hsmp), in1, 0);  if(!b1) return -3;
	hsm_compute_spectrum(b1);


	if(!strcmp(p.file_input2,"")) {
		p.file_input2 = p.file_input1;
		p.hsmp.debug_true_x_valid = 1;
		p.hsmp.debug_true_x[0] = 20;
		p.hsmp.debug_true_x[1] = 50;
		p.hsmp.debug_true_x[2] = 0; /*deg2rad(40.0);*/
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

	char filename_ht1[256]; sprintf(filename_ht1, "%s_ht1.pgm", p.prefix);
	char filename_ht2[256]; sprintf(filename_ht2, "%s_ht2.pgm", p.prefix);
	char filename_hs1[256]; sprintf(filename_hs1, "%s_hs1.pgm", p.prefix);
	char filename_hs2[256]; sprintf(filename_hs2, "%s_hs2.pgm", p.prefix);
	char filename_hs_xc[256]; sprintf(filename_hs_xc, "%s_hs_xc.pgm", p.prefix);
	
	FILE * file_ht1 = open_file_for_writing(filename_ht1);
	FILE * file_ht2 = open_file_for_writing(filename_ht2);
	FILE * file_hs1 = open_file_for_writing(filename_hs1);
	FILE * file_hs2 = open_file_for_writing(filename_hs2);
	FILE * file_hs_xc = open_file_for_writing(filename_hs_xc);
	
	if(!file_ht1 | !file_ht2) return -4;
	if(!file_hs1 | !file_hs2) return -4;
	if(!file_hs_xc) return -5;
	
	write_ht_on_image(b1,file_ht1);
	write_ht_on_image(b2,file_ht2);
	write_function_on_image(b1->num_angular_cells, b1->hs, 200, file_hs1);
	write_function_on_image(b2->num_angular_cells, b2->hs, 200, file_hs2);
	write_function_on_image(b1->num_angular_cells, b1->hs_cross_corr, 200, file_hs_xc);
	

}


void write_function_on_image(int n, const double*f, int rows, FILE*out) {	
	int cols = n;
	gray ** grays = pgm_allocarray(cols, rows);
	
	double maxvalue=0;
	for(int i=0;i<n;i++)
		if(f[i]>0) /* case NAN */
		maxvalue = GSL_MAX(maxvalue, f[i]);
	if(maxvalue==0)maxvalue=1;

	gray maxgray = 255;

	for(int y=0;y<rows;y++)
	for(int x=0;x<cols;x++)
	grays[y][x] = 0;
	
	for(int x=0;x<cols;x++) {
		int y =  round((rows-2)*f[x]/maxvalue);
		y = (rows-1) - y;
		if(y>=0 && y<rows )
		grays[y][x] = maxgray;
	}
	
	pgm_writepgm(out,grays,cols,rows,maxgray,0);
	
	pgm_freearray(grays,rows);
}


void write_ht_on_image(hsm_buffer b, FILE*out) {	
	int rows = b->num_angular_cells;
	int cols = b->num_linear_cells;
	gray ** grays = pgm_allocarray(cols, rows);
	
	double maxvalue=0;
	for(int t=0;t<b->num_angular_cells;t++)
	for(int r=0;r<b->num_linear_cells;r++)
	maxvalue = GSL_MAX(maxvalue, b->ht[t][r]);

	gray maxgray = 255;

	for(int t=0;t<b->num_angular_cells;t++)
	for(int r=0;r<b->num_linear_cells;r++)
	grays[t][r] = (gray) ceil(b->ht[t][r] * (maxgray-1) / maxvalue);
	
	pgm_writepgm(out,grays,cols,rows,maxgray,0);
	
	pgm_freearray(grays,rows);
}

hsm_buffer create_ht_for_image(struct hsm_params*p, FILE*in, const double base[3]) {
	int cols, rows; gray max;
	gray **image = pgm_readpgm(in, &cols, &rows, &max);
	if(!image) { return 0; }

	p->max_norm = 1.1 * hypot(cols/2.0, rows/2.0);
	hsm_buffer b = hsm_buffer_alloc(p);

	/** Add base displacement if specified */
	if(base)
		hsm_compute_ht_base(b, base);
	
	int npoints = 0;
   for (int v=0; v<rows; v++)
	for (int u=0; u<cols; u++) {
		double x = u - cols/2;
		double y = v - rows/2;
		if(image[v][u]==0) continue;
		hsm_compute_ht_point(b, x, y, ((double)image[v][u])/max);
		npoints ++;
	}
	sm_debug("Used %d points.\n", npoints);
    /* write the modified image to stdout */
/*    pgm_writepgm(stdout, image, cols, rows, max, 1); */
    pgm_freearray(image, rows);
	return b;
}
