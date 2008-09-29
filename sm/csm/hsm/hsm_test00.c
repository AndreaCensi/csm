#include <pgm.h>
#include <options/options.h>

#include <csm/csm_all.h>

#include "hsm.h"


const char * banner = 
	"Reads an image, computes HT, de-computes it \n\n";

struct {
	const char * file_input1;
	const char * file_input2;
	
	const char * prefix;
	int debug;
	
	struct hsm_params hsmp;
} p;

hsm_buffer create_ht_for_image(struct hsm_params*p, FILE*in);
void write_ht_on_image(hsm_buffer b, FILE*out);
void write_function_on_image(int n, const double*f, int rows, FILE*out);


int main(int argc, const char**argv) {
	pgm_init(&argc, argv);
	options_banner(banner);
	
	
	struct option* ops = options_allocate(10);
	options_string(ops, "in1", &p.file_input1, "stdin", "Input file 1");
	options_string(ops, "in2", &p.file_input2, "stdin", "Input file 2");
	options_string(ops, "out", &p.prefix, "test00", "Output file prefix ");
	options_double(ops, "hsm_linear_cell_size", &p.hsmp.linear_cell_size, 1.0, "Size of a rho cell");
	options_double(ops, "hsm_angular_cell_size_deg", &p.hsmp.angular_cell_size_deg, 1.0, "Size of angualar cell (deg)");

	options_int(ops, "debug", &p.debug, 0, "Shows debug information");
	
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}
	
	FILE * in1 = open_file_for_reading(p.file_input1);
	FILE * in2 = open_file_for_reading(p.file_input2);
	if(!in1 | !in2 ) return -2;

	hsm_buffer b1 = create_ht_for_image(&(p.hsmp), in1);
	hsm_buffer b2 = create_ht_for_image(&(p.hsmp), in2);

	if(!b1 | !b2) return -3;
	
	
	hsm_compute_spectrum(b1);
	hsm_compute_spectrum(b2);
	
/*	hsm_match(&(p.hsmp),b1,b2);*/

	char filename_ht1[256]; sprintf(filename_ht1, "%s_ht1.pgm", p.prefix);
	char filename_ht2[256]; sprintf(filename_ht2, "%s_ht2.pgm", p.prefix);
	char filename_hs1[256]; sprintf(filename_hs1, "%s_hs1.pgm", p.prefix);
	char filename_hs2[256]; sprintf(filename_hs2, "%s_hs2.pgm", p.prefix);
	char filename_hs_xc[256]; sprintf(filename_hs_xc, "%s_hs1.pgm", p.prefix);
	
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
	

/*	struct hsm_result * res = hsm_matching(&p, b1, b2);
	
	hsm_result_free(res);*/
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

hsm_buffer create_ht_for_image(struct hsm_params*p, FILE*in) {
	int cols, rows; gray max;
	gray **image = pgm_readpgm(in, &cols, &rows, &max);
	if(!image) { return 0; }

	p->max_norm = 1.1 * hypot(cols/2.0, rows/2.0);
	hsm_buffer b = hsm_buffer_alloc(p);
	
   for (int v=0; v<rows; v++)
	for (int u=0; u<cols; u++) {
		double x = u - cols/2;
		double y = v - rows/2;
		if(image[v][u]==0) continue;
		hsm_compute_ht_point(b, x, y, ((double)image[v][u])/max);
	}
    /* write the modified image to stdout */
/*    pgm_writepgm(stdout, image, cols, rows, max, 1); */
    pgm_freearray(image, rows);
	return b;
}
