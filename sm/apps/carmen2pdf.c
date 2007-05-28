#include <time.h>

#include <cairo.h>
#include <cairo-pdf.h>

#include "../lib/options/options.h"


#include "../src/math_utils.h"
#include "../src/sm.h"
#include "../src/laser_data.h"

struct params {
	int interval;
	int use_odometry;
	double horizon;
	double line_threshold;
	double dimension;
	const char*output_filename;
	const char*input_filename;

	FILE*input_file;
};

void carmen2pdf(struct params p);

void ld_getbb(struct laser_data* ld, double*x0, double*y0, double*x1, double*y1, 
	int use_odometry, double horizon);

int main(int argc, const char*argv[]) {
	struct params p;
	p.interval = 10;
	
	p.input_filename = "in.log";
	p.output_filename = "out.pdf";
	p.use_odometry = 0;
	p.line_threshold = 0.2;
	p.horizon = 8;
	p.dimension=500;
	
	struct option options[8] = 
		{ {"--interval",  "how many to ignore",                   OPTION_INT,    &(p.interval), 0},
		  {"--in",        "input filename (Carmen format)",       OPTION_STRING, &(p.input_filename), 0},
		  {"--out",       "output filename (pdf)",                OPTION_STRING, &(p.output_filename), 0},
		  {"--lt",        "threshold for linking points (m)",     OPTION_DOUBLE, &(p.line_threshold), 0},
		  {"--horizon",   "horizon of the laser (m)",             OPTION_DOUBLE, &(p.horizon), 0},
		  {"--dimension", "dimension of image (points)",          OPTION_DOUBLE, &(p.dimension), 0},
		  {"--use_odometry", "if 1 uses odometry, else estimate", OPTION_INT,    &(p.use_odometry), 0},
		  {0,0,0,0,0}};
	
	if(!options_parse_args(options, argc, argv)) {
		printf("error.");
		options_print_help(options, stderr);
		return -1;
	}
	
	p.input_file = fopen(p.input_filename,"r");
	if(p.input_file==NULL) {
		fprintf(stderr, "Could not open '%s'.\n", p.input_filename); 
		return -1;
	}
	
	carmen2pdf(p);
	return 0;
}

int should_consider(struct params *p, int counter) {
	return counter%p->interval == 0;
}

void ld_get_world(struct laser_data*ld, int i, double*x, double*y,int use_odometry);

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

void get_bb(struct params*p, struct bounding_box*bb) {
	struct laser_data ld;	
	int counter=0;
	if(!ld_read_next_laser_carmen(p->input_file, &ld)) {
		ld_getbb(&ld,&bb->x0,&bb->y0,&bb->x1,&bb->y1,p->use_odometry, p->horizon);
	}
	ld_dealloc(&ld);
	while(!ld_read_next_laser_carmen(p->input_file, &ld)) {
		if(should_consider(p, counter))  {
			double x0,y0,x1,y1;
			ld_getbb(&ld,&x0,&y0,&x1,&y1,p->use_odometry, p->horizon);
			bb->x0 = GSL_MIN(x0, bb->x0);
			bb->x1 = GSL_MAX(x1, bb->x1);
			bb->y0 = GSL_MIN(y0, bb->y0);
			bb->y1 = GSL_MAX(y1, bb->y1);
		}
		counter++;
		ld_dealloc(&ld);
	}
	rewind(p->input_file);
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
	
	struct laser_data ld;	
	int counter=0; 
	int first_pose=1; double old_pose_bx,old_pose_by;
	while(!ld_read_next_laser_carmen(p.input_file, &ld)) {
	
		{
			double bx,by;
			if(p.use_odometry)
				bb_w2b(&bb, ld.odometry[0], ld.odometry[1], &bx,&by);
			else
				bb_w2b(&bb, ld.estimate[0], ld.estimate[1], &bx,&by);

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
			int i; for(i=0;i<ld.nrays;i++) {
				if(ld.valid[i]==0) continue;
				if(ld.readings[i]>p.horizon) continue;
				
				double x,y;
				ld_get_world(&ld, i, &x, &y, p.use_odometry);
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
		ld_dealloc(&ld);
	}

	cairo_show_page (cr);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
}

void ld_get_world(struct laser_data*ld, int i, double*x, double*y, int use_odometry) {
	gsl_vector * p = gsl_vector_alloc(2);
	gsl_vector * pw = gsl_vector_alloc(2);
	gsl_vector * pose = gsl_vector_alloc(3);
	
	gsl_vector_set(p, 0, cos(ld->theta[i]) * ld->readings[i]);
	gsl_vector_set(p, 1, sin(ld->theta[i]) * ld->readings[i]);
	
	if(use_odometry)
		copy_from_array(pose, ld->odometry);
	else
		copy_from_array(pose, ld->estimate);
	
	transform(p, pose, pw);
	
	*x = gsl_vector_get(pw, 0);
	*y = gsl_vector_get(pw, 1);

	gsl_vector_free(p);
	gsl_vector_free(pw);
	gsl_vector_free(pose);
}

void ld_getbb(struct laser_data* ld, double*x0, double*y0, double*x1, double*y1,
 	int use_odometry, double horizon) {
	
	int first=1;
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld->valid[i]) continue;
		if(ld->readings[i]>horizon) continue;
		double x,y;
		ld_get_world(ld, i, &x, &y, use_odometry);
		
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




