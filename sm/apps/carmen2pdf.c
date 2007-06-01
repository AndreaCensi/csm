#include <time.h>
#include <string.h>
#include <cairo.h>
#include <cairo-pdf.h>

#include <options/options.h>


#include "../src/math_utils.h"
#include "../src/sm.h"
#include "../src/laser_data.h"
#include "../src/laser_data_json.h"
#include "../src/utils.h"
#include "../src/logging.h"

typedef enum { Odometry = 1, Estimate = 2, True_pose = 3 } reference;
const char*reference_name[4] = { "invalid","odometry","estimate","true_pose"};

struct params {
	int interval;
	const char*use;
	double padding;
	double horizon;
	double line_threshold;
	double dimension;
	const char*output_filename;
	const char*input_filename;

	/**/
	FILE*input_file;
	reference use_reference;
};

void carmen2pdf(struct params p);

void ld_getbb(LDP  ld, double*x0, double*y0, double*x1, double*y1, 
	reference use_reference, double horizon);

int main(int argc, const char*argv[]) {
	struct params p;
	
	struct option * ops = options_allocate(10);
	options_int(ops, "interval", &p.interval, 10, "how many to ignore");
	options_string(ops, "in", &p.input_filename, "stdin", "input file (Carmen or JSON)");
	options_string(ops, "out", &p.output_filename, "carmen2pdf.pdf", "output file (PDF)");
	options_double(ops, "lt", &p.line_threshold, 0.2, "threshold for linking points (m)");
	options_double(ops, "horizon", &p.horizon, 8.0, "horizon of the laser (m)");
	options_double(ops, "padding", &p.padding, 0.2, "padding around bounding box (m)");
	options_double(ops, "dimension", &p.dimension, 500.0, "dimension of the image (points)");
	options_string(ops, "use", &p.use, "odometry", "One in 'odometry','estimate','true_pose'");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "error.");
		options_print_help(ops, stderr);
		return -1;
	}
	
	p.use_reference = 0;
	int i; for(i=1;i<=3;i++) 
		if(!strcmp(p.use, reference_name[i]))
			p.use_reference = (reference) i;
	if(0 == p.use_reference) {
		fprintf(stderr, "Invalid reference '%s'. " 
			"Use one in 'odometry','estimate','true_pose'.\n", p.use);
		return -1;
	}
	
	p.input_file = open_file_for_reading(p.input_filename);
	if(!p.input_file) return -1;
	
	carmen2pdf(p);
	return 0;
}

int should_consider(struct params *p, int counter) {
	return counter%p->interval == 0;
}

void ld_get_world(LDP ld, int i, double*x, double*y, reference use_reference);

struct bounding_box {
	/** World frame */
	double x0,y0,x1,y1;
	/* Paper size */
	double width, height;
};

void bb_w2b(struct bounding_box*bb, double wx, double wy, double*bx, double*by){
	double scale = GSL_MIN(bb->width / (bb->x1-bb->x0), bb->height / (bb->y1-bb->y0));
	*bx = (wx-bb->x0) * scale;
	*by = bb->height- (wy-bb->y0) * scale;
}

/** Reads all file to find bounding box */
void get_bb(struct params*p, struct bounding_box*bb) {
	LDP ld;	
	int counter=0;

	if((ld = ld_read_smart(p->input_file))) {
		ld_getbb(ld,&bb->x0,&bb->y0,&bb->x1,&bb->y1,p->use_reference, p->horizon);
		ld_free(ld);
	}
	
	while((ld = ld_read_smart(p->input_file))) {
		if(should_consider(p, counter))  {
			double x0,y0,x1,y1;
			ld_getbb(ld,&x0,&y0,&x1,&y1,p->use_reference, p->horizon);
			bb->x0 = GSL_MIN(x0, bb->x0);
			bb->x1 = GSL_MAX(x1, bb->x1);
			bb->y0 = GSL_MIN(y0, bb->y0);
			bb->y1 = GSL_MAX(y1, bb->y1);
		}
		counter++;
		ld_free(ld);
	}
	rewind(p->input_file);
	
	bb->x0 -= p->padding;
	bb->x1 += p->padding;
	bb->y0 -= p->padding;
	bb->y1 += p->padding;
}

int any_nan(double *d, int n) {
	int i; for(i=0;i<n;i++) 
		if(is_nan(d[i]))
		return 1;
	return 0;
}

double * ld_get_reference(LDP ld, reference use_reference) {
	double * pose;
	switch(use_reference) {
		case Odometry: pose = ld->odometry; break;
		case Estimate: pose = ld->estimate; break;
		case True_pose: pose = ld->true_pose; break;
	}
	if(any_nan(pose, 3)) {
		sm_error("Required field '%s' not set in laser scan.\n", 
			reference_name[use_reference] );
		sm_error("I will abruptly exit() because of a panic attack.\n");
		exit(-1);
	}
	return pose;
}

void carmen2pdf(struct params p) {
	
	struct bounding_box bb;
	get_bb(&p, &bb);

	double wwidth = bb.x1-bb.x0;
	double wheight= bb.y1-bb.y0;
	if(wwidth > wheight) {
		bb.width = p.dimension;
		bb.height = bb.width / wwidth * wheight;
	} else {
		bb.height = p.dimension;
		bb.width = bb.height / wheight * wwidth;
	}
	
	printf("Bounding box: %f %f, %f %f\n",bb.x0,bb.y0,bb.x1,bb.y1);
	
	cairo_surface_t *surface;
	cairo_t *cr;
	cairo_status_t status;
	
	surface = cairo_pdf_surface_create(p.output_filename, bb.width, bb.height);
	cr = cairo_create (surface);
	status = cairo_status (cr);

	if (status) {
		printf("Failed to create pdf surface for file %s: %s\n",
			p.output_filename, cairo_status_to_string (status));
		return;
	}
	

/*	double scale = GSL_MIN(bb.width / (bb.x1-bb.x0), bb.height / (bb.y1-bb.y0));

	cairo_scale(cr, 50, 1);
	cairo_translate(cr, 0, -0.5*bb.height); */
	
	int counter=0; 
	int first_pose=1; double old_pose_bx,old_pose_by;
	LDP ld;
	while((ld = ld_read_smart(p.input_file))) {
	
		{
			double bx,by;
			
			double * pose = ld_get_reference(ld, p.use_reference);
			bb_w2b(&bb, pose[0], pose[1], &bx, &by);

			if(first_pose) { first_pose = 0; 
			} else {
				cairo_set_line_width(cr, 0.5);
				cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
				cairo_move_to(cr, old_pose_bx, old_pose_by);
				cairo_line_to(cr, bx, by);
				cairo_close_path(cr);
				cairo_stroke(cr);
			}

			old_pose_bx = bx;
			old_pose_by = by;
		}
		
		if(should_consider(&p, counter))  {
			cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
			cairo_set_line_width(cr, 0.1);
			int first = 1;
			double last_x,last_y;
			int i; for(i=0;i<ld->nrays;i++) {
				if(ld->valid[i]==0) continue;
				if(ld->readings[i]>p.horizon) continue;
				
				double x,y;
				ld_get_world(ld, i, &x, &y, p.use_reference);
				double bx,by;
				bb_w2b(&bb, x,y,&bx,&by);
				
				if(first) { first = 0;
					cairo_move_to(cr, bx, by);
					last_x=x; last_y=y;
				} else {
					int near = square(p.line_threshold) > 
						square(x-last_x)+square(y-last_y);
					
					if(near) {
						cairo_line_to(cr, bx, by);
					} else {
						cairo_close_path(cr);
						cairo_stroke(cr);
						cairo_move_to(cr, bx, by);
						last_x=x; last_y=y;
					}
				}
				
			}
			cairo_close_path(cr);
			cairo_stroke(cr);
		}
		counter++;
		ld_free(ld);
	}

	cairo_show_page (cr);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
}

void ld_get_world(LDP ld, int i, double*x, double*y, reference use_reference) {
	gsl_vector * p = gsl_vector_alloc(2);
	gsl_vector * pw = gsl_vector_alloc(2);
	gsl_vector * pose = gsl_vector_alloc(3);
	
	gsl_vector_set(p, 0, cos(ld->theta[i]) * ld->readings[i]);
	gsl_vector_set(p, 1, sin(ld->theta[i]) * ld->readings[i]);
	
	copy_from_array(pose, ld_get_reference(ld, use_reference));
	
	transform(p, pose, pw);
	
	*x = gsl_vector_get(pw, 0);
	*y = gsl_vector_get(pw, 1);

	gsl_vector_free(p);
	gsl_vector_free(pw);
	gsl_vector_free(pose);
}

void ld_getbb(LDP  ld, double*x0, double*y0, double*x1, double*y1,
 	reference use_reference, double horizon) {
	
	int first=1;
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld->valid[i]) continue;
		if(ld->readings[i]>horizon) continue;
		double x,y;
		ld_get_world(ld, i, &x, &y, use_reference);
		
		if(first) {
			*x0 = *x1 = x;
			*y0 = *y1 = y;
			first = 0;
		} else {
			*x0 = GSL_MIN(*x0, x);
			*y0 = GSL_MIN(*y0, y);
			*x1 = GSL_MAX(*x1, x);
			*y1 = GSL_MAX(*y1, y);
		}
	}
}




