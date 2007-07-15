
#include "ld_cairo.h"

void cr_ld_draw_rays(cairo_t*, LDP, line_style*);
void cr_ld_draw_countour(cairo_t*, LDP, line_style*);
void cr_ld_draw_points(cairo_t*, LDP, line_style*, float radius);


void ls_add_options(line_style*ls, struct option*ops, 
	const char*prefix, const char*desc_prefix) 
{
		
}


void lds_add_options(ld_style*lds, struct option*ops, 
	const char*prefix, const char*desc_prefix) 
{
	
}

void cr_set_style(cairo_t*cr, line_style*ls) {
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	
	cairo_set_line_width(cr, ls->width);
}

void cr_ld_draw_rays(cairo_t*cr, LDP ld, line_style*ls){
	cr_set_style(cr, ls);
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld_valid_ray(ld, i)) continue;
		
		double threshold = 2;
		if(ld->readings[i]<2) continue;
		
		double x1 = threshold * cos(ld->theta[i]);
		double y1 = threshold * sin(ld->theta[i]);
		double x2 = ld->readings[i] * cos(ld->theta[i]);
		double y2 = ld->readings[i] * sin(ld->theta[i]);
		
		cairo_move_to(cr,x1,y1);
		cairo_line_to(cr,x2,y2);
		cairo_stroke(cr);
	}
}

struct stroke_sequence {
	int begin_new_stroke;
	int end_stroke;
	int valid;
}; 

void compute_stroke_sequence(LDP ld, struct stroke_sequence*draw_info,
	double horizon){
	int last_valid = -1; int first = 1;
	int i; for(i=0;i<ld->nrays;i++) {
		if( (!ld_valid_ray(ld,i)) || (ld->readings[i] > horizon) ) {
			draw_info[i].valid = 0;
			continue;
		}
		draw_info[i].valid = 1;

		if(first) { 
			first = 0; 
			draw_info[i].begin_new_stroke = 1;
			draw_info[i].end_stroke = 0;
		} else {
			int near = 1; /* XXX square(p.line_threshold) > 
				distance_squared_d(draw_info[last_valid].w, draw_info[i].w);*/
			draw_info[i].begin_new_stroke = near ? 0 : 1;
			draw_info[i].end_stroke = 0;
			draw_info[last_valid].end_stroke = draw_info[i].begin_new_stroke;
		}
		last_valid = i;
	} /*for */
	if(last_valid >= 0)
		draw_info[last_valid].end_stroke = 1;
} /* find buff .. */

void cr_ld_draw_countour(cairo_t*cr, LDP ld, line_style*ls) {
	struct stroke_sequence draw_info[ld->nrays];
	compute_stroke_sequence(ld, draw_info, 10);
	
	/* draw contour: begin_new_stroke and end_stroke tell 
	when to interrupt the stroke */
	int i; 
	cr_set_style(cr, ls);
	for(i=0;i<ld->nrays;i++) {

		if(draw_info[i].valid==0) continue;

		double x = ld->readings[i] * cos(ld->theta[i]);
		double y = ld->readings[i] * sin(ld->theta[i]);
		
		if(draw_info[i].begin_new_stroke)
			cairo_move_to(cr, x, y);
		else
			cairo_line_to(cr, x, y);
		if(draw_info[i].end_stroke)
			cairo_stroke(cr);
	}
}

void cr_ld_draw_points(cairo_t*cr, LDP ld, line_style*ls, float radius) {
	cr_set_style(cr, ls);
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld_valid_ray(ld, i)) continue;

		double x = ld->readings[i] * cos(ld->theta[i]);
		double y = ld->readings[i] * sin(ld->theta[i]);

		cairo_arc (cr, x, y, radius, 0, 2*M_PI);
		cairo_stroke (cr);
	}
}

void cr_ld_draw(cairo_t* cr, LDP ld, ld_style *p) {
	if(p->rays.draw) 
		cr_ld_draw_rays(cr, ld, &(p->rays));
		
	if(p->countour.draw)  
		cr_ld_draw_countour(cr, ld, &(p->countour));

	if(p->points.draw) 
		cr_ld_draw_points(cr, ld, &(p->points), p->points_radius);
}

void cr_set_reference(cairo_t*cr, double*pose) {
	cairo_rotate(cr,pose[2]);
	cairo_translate(cr,pose[0],pose[1]);
}
