#include <cairo/cairo.h>
#include <options/options.h>
#include "../csm/csm_all.h"

typedef struct {
	/* should we draw it? */
	int draw;
	
	double width;
	const char* color;
/*	
	int	thickness; //		(1/80 inch)
	int	pen_color; //		(enumeration type, pen color)
	int	fill_color; //		(enumeration type, fill color)
	int	pen_style; //		(pen style, not used)
	int	area_fill; //		(enumeration type, -1 = no fill)
//	float	style_val; //		(1/80 inch)
*/
} line_style ;

typedef struct { 
	line_style rays, countour;
	line_style points;
	double points_radius; 

	line_style normals;
	double normals_length;
	
	double connect_threshold;
	double horizon;
} ld_style;

void ls_set_defaults(line_style*ls);
void lds_set_defaults(ld_style*lds);

void ls_add_options(line_style*ls, struct option*ops, 
	const char*prefix, const char*desc_prefix);

void lds_add_options(ld_style*lds, struct option*ops, 
	const char*prefix, const char*desc_prefix);

void cr_set_style(cairo_t*cr, line_style*);
void cr_ld_draw(cairo_t* cr, LDP ld, ld_style *p);
void cr_ld_draw_corr(cairo_t*cr, LDP laser_ref, LDP laser_sens, line_style*);

void cr_set_reference(cairo_t*cr,double*pose);

/** Needs cartesian; returns 0 if not enough points. */
/*int ld_get_bounding_box(LDP ld, double min[2], double max[2], double horizon);*/




