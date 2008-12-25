#include <time.h>
#include <string.h>

#ifdef LINUX
#include <linux/limits.h>
#endif


#include <cairo.h>
#include <cairo-pdf.h>

#include <options/options.h>

#include "../csm/csm_all.h"


typedef enum { Invalid = 0, Odometry = 1, Estimate = 2, True_pose = 3 } reference;
const char*reference_name[4] = { "invalid","odometry","estimate","true_pose"};

struct params {
	int interval;
	const char*use;
	double padding;
	double horizon;
	double line_threshold;
	double dimension;
	
	int draw_confidence;
	double confidence_mult;
	
	const char*output_filename;
	const char*input_filename;

	/**/
	FILE*input_file;
	reference use_reference;

	double offset_theta_deg;
};

void carmen2pdf(struct params p);

double offset_theta = 0;
void ld_getbb(LDP  ld, double*x0, double*y0, double*x1, double*y1, 
	reference use_reference, double horizon);

int main(int argc, const char*argv[]) {
	sm_set_program_name(argv[0]);
	fprintf(stderr, "carmen2pdf:\t *** Please use log2pdf instead. ***\n\n");

	struct params p;
	
	struct option * ops = options_allocate(12);
	options_int(ops, "interval", &p.interval, 10, "how many to ignore");
	options_string(ops, "in", &p.input_filename, "stdin", "input file (Carmen or JSON)");
	options_string(ops, "out", &p.output_filename, "", "output file (if empty, input file + '.pdf')");
	options_double(ops, "lt", &p.line_threshold, 0.2, "threshold for linking points (m)");
	options_double(ops, "horizon", &p.horizon, 8.0, "horizon of the laser (m)");
	options_double(ops, "padding", &p.padding, 0.2, "padding around bounding box (m)");
	options_double(ops, "dimension", &p.dimension, 500.0, "dimension of the image (points)");
	options_int(ops, "draw_confidence", &p.draw_confidence, 0, " Draws confidence (readings_sigma[i]) ");
	options_double(ops, "confidence_mult", &p.confidence_mult, 3.0, " 3-sigma ");
	options_double(ops, "offset_theta_deg", &p.offset_theta_deg, 0.0, " rotate entire map by this angle (deg) ");

	options_string(ops, "use", &p.use, "estimate", "One in 'odometry','estimate','true_pose'");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_error("Could not parse arguments.\n");
		options_print_help(ops, stderr);
		return -1;
	}
	
	/* If out not specified */
	if(strlen(p.output_filename)==0) {
		char buf[PATH_MAX];
		sprintf(buf, "%s.pdf", p.input_filename);
		p.output_filename = strdup(buf);
		sm_info("Writing on file '%s'.\n", p.output_filename);
	}
	
	p.use_reference = Invalid;
	int i; for(i=1;i<=3;i++) 
		if(!strcmp(p.use, reference_name[i]))
			p.use_reference = (reference) i;
	if(Invalid == p.use_reference) {
		sm_error("Invalid reference '%s'. " 
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

void ld_get_buffer_polar(double phi, double rho, const double*pose, 
	double*x, double*y,
	struct bounding_box*bb, double*bx,double *by);


/** Reads all file to find bounding box */
void get_bb(struct params*p, struct bounding_box*bb) {
	LDP ld;
	int counter = -1, 
	    considered = 0;

	while((ld = ld_read_smart(p->input_file))) {
		counter++;
		if(should_consider(p, counter))  {
			if(!ld_valid_fields(ld))  {
				sm_error("Invalid laser data (#%d in file)\n", counter);
				continue;
			}
			
			double x0,y0,x1,y1;
			ld_getbb(ld,&x0,&y0,&x1,&y1, p->use_reference, p->horizon);
			if(considered > 0) {
				bb->x0 = GSL_MIN(x0, bb->x0);
				bb->x1 = GSL_MAX(x1, bb->x1);
				bb->y0 = GSL_MIN(y0, bb->y0);
				bb->y1 = GSL_MAX(y1, bb->y1);
			} else {
				/* this is the first one */
				bb->x0 = x0;
				bb->x1 = x1;
				bb->y0 = y0;
				bb->y1 = y1;
			}
			
			considered++;
		}
		ld_free(ld);
	}
	sm_info("Considering %d of %d scans.\n", considered, counter+1);
	rewind(p->input_file);
	
	bb->x0 -= p->padding;
	bb->x1 += p->padding;
	bb->y0 -= p->padding;
	bb->y1 += p->padding;
}


double * ld_get_reference(LDP ld, reference use_reference) {
	double * pose;
	switch(use_reference) {
		case Odometry: pose = ld->odometry; break;
		case Estimate: pose = ld->estimate; break;
		case True_pose: pose = ld->true_pose; break;
		default: exit(-1);
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
	
	offset_theta += deg2rad(p.offset_theta_deg);

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
	
	sm_info("Bounding box: %f %f, %f %f\n",bb.x0,bb.y0,bb.x1,bb.y1);
	
	cairo_surface_t *surface;
	cairo_t *cr;
	cairo_status_t status;
	
	surface = cairo_pdf_surface_create(p.output_filename, bb.width, bb.height);
	cr = cairo_create (surface);
	status = cairo_status (cr);

	if (status) {
		sm_error("Failed to create pdf surface for file %s: %s\n",
			p.output_filename, cairo_status_to_string (status));
		return;
	}
	
	int counter=0; 
	int first_pose=1; double old_pose_bx=0,old_pose_by=0;
	LDP ld;
	while((ld = ld_read_smart(p.input_file))) {
	
		double *pose = ld_get_reference(ld, p.use_reference);

		/* Draw pose */
		{
			double bx,by;
			ld_get_buffer_polar(0.0,0.0,pose, 0,0, &bb, &bx, &by);
			if(first_pose) { 
				first_pose = 0; 
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
		

		/* If should we draw this sensor scan */
		if(should_consider(&p, counter))  {

			/* Firstly, find buffer coordinates and whether to cut the stroke */
			struct {
				double w[2]; /* world coordinates */
				double b[2]; /* buffer coordinates */
				int begin_new_stroke;
				int end_stroke;
				int valid;
				} draw_info[ld->nrays];
			
			{
				int last_valid = -1; int first = 1;
				int i; for(i=0;i<ld->nrays;i++) {

					if( (!ld->valid[i]) || ld->readings[i]>p.horizon) {
						draw_info[i].valid = 0;
						continue;
					}
					draw_info[i].valid = 1;

					ld_get_buffer_polar(ld->theta[i], ld->readings[i], 
						pose, &(draw_info[i].w[0]), &(draw_info[i].w[1]), 
						&bb,  &(draw_info[i].b[0]), &(draw_info[i].b[1]));

					if(first) { 
						first = 0; 
						draw_info[i].begin_new_stroke = 1;
						draw_info[i].end_stroke = 0;
					} else {
						int near = square(p.line_threshold) > 
							distance_squared_d(draw_info[last_valid].w, draw_info[i].w);
						draw_info[i].begin_new_stroke = near ? 0 : 1;
						draw_info[i].end_stroke = 0;
						draw_info[last_valid].end_stroke = draw_info[i].begin_new_stroke;
					}
					last_valid = i;
				} /*for */
				if(last_valid >= 0)
					draw_info[last_valid].end_stroke = 1;
			} /* find buff .. */
			

			if(p.draw_confidence) { 
				int i;
				/* Compute interval */
				double interval[ld->nrays];
				double big_interval = 0.3;
				for(i=0;i<ld->nrays;i++) { if(draw_info[i].valid==0) continue;
					double sigma = ld->readings_sigma[i];
					if(!is_nan(cov)) {
						interval[i] = p.confidence_mult * sigma;
					} else interval[i] = big_interval;
				}

				cairo_set_source_rgb(cr, 1.0, 0.5, 0.5);
				cairo_set_line_width(cr, 0.1);
				/* draw one */
				int j=0; for(j=0;j<2;j++)
				for(i=0;i<ld->nrays;i++) { if(draw_info[i].valid==0) continue;
					double b[2];
					ld_get_buffer_polar(ld->theta[i], 
						ld->readings[i] + (j ? interval[i] : -interval[i]), 
						pose, 0, 0, &bb,  &(b[0]), &(b[1]));

					if(draw_info[i].begin_new_stroke)
						cairo_move_to(cr, b[0], b[1]);
					else
						cairo_line_to(cr, b[0], b[1]);
					if(draw_info[i].end_stroke)
						cairo_stroke(cr);
				}
			} /* draw confidence */
			
			/* draw contour: begin_new_stroke and end_stroke tell 
			when to interrupt the stroke */
			int i; 
			cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
			cairo_set_line_width(cr, 0.5);
			for(i=0;i<ld->nrays;i++) {
				if(draw_info[i].valid==0) continue;
				double *b = draw_info[i].b;
				if(draw_info[i].begin_new_stroke)
					cairo_move_to(cr, b[0], b[1]);
				else
					cairo_line_to(cr, b[0], b[1]);
				if(draw_info[i].end_stroke)
					cairo_stroke(cr);
			}

			
			
		}
		counter++;
		ld_free(ld);
	}

	cairo_show_page (cr);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
}

void ld_get_buffer_polar(double phi, double rho, const double*pose, 
	double*x, double*y,
	struct bounding_box*bb, double*bx,double *by) {
	
	double point[2];
	point[0] = cos(phi) * rho;
	point[1] = sin(phi) * rho;

	double frame[3] = { 0, 0, offset_theta};
	double pose2[3];
	oplus_d(frame, pose, pose2);
	double pw[2];
	transform_d(point, pose2, pw);
	
	if( (bb!=0) & (bx!=0) & (by!=0) )
	bb_w2b(bb, pw[0], pw[1],  bx, by);
	
	if((x!=0) && (y!=0)) {
		*x = pw[0]; *y = pw[1];
	}
}

void ld_getbb(LDP  ld, double*x0, double*y0, double*x1, double*y1,
 	reference use_reference, double horizon) {
	double *pose = ld_get_reference(ld, use_reference);
	
	int nrays_used = 0;
	int first=1;
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld->valid[i]) continue;
		if(ld->readings[i]>horizon) continue;
		double x,y;
		ld_get_buffer_polar(ld->theta[i], ld->readings[i], pose, &x, &y, 0, 0,0);

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
		nrays_used++;
	}
}




