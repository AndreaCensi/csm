#include <cairo/cairo.h>

typedef struct {
	double line_width;
	char   line_color[10];

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
	double points_radius; 

} ld_style;


void add_options(line_style*, struct option*ops, const char*prefix, const char*desc_prefix)
void add_options(ld_style*, struct option*ops, const char*prefix, const char*desc_prefix);
void line_style_set(line_style*, cairo_t*);
