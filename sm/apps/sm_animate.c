#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../csm/csm_all.h"

#include <options/options.h>
#include <cairo-pdf.h>

#include "ld_cairo.h"

typedef struct {
	const char * file_input;
	const char * file_output;
	
	ld_style laser_ref_s, laser_sens_s;
	/* Drawing style for correspondences */
	line_style corr;
} anim_params ;

int draw_animation( anim_params* p, JO jo);


void ls_set_defaults(line_style*ls) {
	ls->draw = 1;
	sprintf(ls->color, "black");
	ls->width = 0.002;
}

void lds_set_defaults(ld_style*lds) {
	ls_set_defaults(&(lds->rays));
	lds->rays.width = 0.0002;
	ls_set_defaults(&(lds->countour));
	ls_set_defaults(&(lds->points));
	lds->points_radius = 0.003;
}

void set_defaults(anim_params *p) {
	lds_set_defaults(&(p->laser_ref_s));
	lds_set_defaults(&(p->laser_sens_s));
	ls_set_defaults(&(p->corr));
}

int main(int argc, const char** argv)
{
	anim_params p;
	set_defaults(&p);
	
	struct option* ops = options_allocate(60);
	options_string(ops, "in", &p.file_input, "stdin", "Input file (defaults to stdin)");
	options_string(ops, "out", &p.file_output, "sm_animate.pdf", "Output file ");


	lds_add_options(&(p.laser_ref_s), ops, "ref", "Ref  |");
	lds_add_options(&(p.laser_sens_s), ops, "sens", "Sens |");
	ls_add_options(&(p.corr), ops, "corr", "Correspondences |");
	
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info("Draws icp animation.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}


	FILE * input = open_file_for_reading(p.file_input);
	if(!input) return -1;
	
	
	JO jo; 
	int count = 0; 	
	while( (jo = json_read_stream(input)) ) {
		if(!draw_animation(&p, jo))
		return 0;
		count++;
	}

}

/** Returns an array with depths */
int draw_animation(anim_params* p, JO jo) {
	JO jo_ref = jo_get(jo, "laser_ref");
	JO jo_sens = jo_get(jo, "laser_sens");
	if(!jo_ref || !jo_sens) {
		sm_error("Could not get laser_ref/laser_sens.\n");
		return 0;
	}
	
	LDP laser_ref = json_to_ld(jo_ref);
	LDP laser_sens = json_to_ld(jo_sens); 
	if(!laser_ref || !laser_sens) {
		sm_error("Could not read laser_ref/laser_sens.\n");
		return 0;
	}

	double max_readings = 0;
	int i; for(i=0;i<laser_ref->nrays;i++) {
		if(!ld_valid_ray(laser_ref,i)) continue;
		if(max_readings < laser_ref->readings[i])
			max_readings = laser_ref->readings[i];
	}
	sm_debug("max_readings: %f", max_readings);

	double points = 500;
	double width_points = points;
	double height_points = points;
	
	
	cairo_surface_t *surface;
	cairo_t *cr;
	cairo_status_t status;
	
	surface = cairo_pdf_surface_create(p->file_output, width_points, height_points);
	cr = cairo_create (surface);
	status = cairo_status (cr);

	if (status) {
		sm_error("Failed to create pdf surface for file %s: %s\n",
			p->file_output, cairo_status_to_string (status));
		return 0;
	}


	cairo_translate(cr, points/2, points/2);
	double scale = 0.5 *(points/max_readings);
	cairo_scale(cr, scale, -scale);
	
	
	cairo_set_line_width(cr, 0.01);
	cairo_set_source_rgb (cr, 1.0, 0.2, 0.2);
	cairo_arc (cr, 0.0, 0.0, max_readings, 0.0, 2*M_PI);
	cairo_stroke (cr);
	cairo_move_to(cr, 0.0, 0.0);
	cairo_line_to(cr, max_readings, 0.0);
	
	JO iterations = jo_get(jo, "iterations");
	if(!iterations || !json_object_is_type(iterations, json_type_array)) {
		fprintf(stderr, "Could not read iterations.\n");
		return 0;
	}
	
	int niterations = json_object_array_length(iterations);
	sm_error("Reading %d iterations.\n", niterations);
	int it;

	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);

	if(niterations>10) niterations=10;
	for(it=0;it<niterations;it++) {
		JO iteration = json_object_array_get_idx(iterations, it);
		
		double x_old[3], x_new[3];
		jo_read_double_array(iteration, "x_old", x_old, 3, NAN);
		jo_read_double_array(iteration, "x_new", x_new, 3, NAN);

		JO corr0 = jo_get(iteration, "corr0");
		JO corr1 = jo_get(iteration, "corr1");
		JO corr2 = jo_get(iteration, "corr2");
		if(!corr1 || !corr2 || !corr0) {
			sm_error("Could not read correspondences (field 'corr<i>').\n");
			return 0;
		}

		cairo_save(cr);
	/*		double zero[3] = {0,0,0};
			cr_set_reference(cr, zero);*/
			cr_ld_draw(cr, laser_ref, &(p->laser_ref_s));

			ld_compute_world_coords(laser_sens, x_old);

			
			cr_set_style(cr, &(p->corr));
			cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
			json_to_corr(corr0, laser_sens->corr, laser_sens->nrays);
			cr_ld_draw_corr(cr, laser_ref, laser_sens, &(p->corr));

			cairo_set_source_rgb (cr, 1.0, 0.0, 1.0);
			json_to_corr(corr1, laser_sens->corr, laser_sens->nrays);
			cr_ld_draw_corr(cr, laser_ref, laser_sens, &(p->corr));

			cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
			json_to_corr(corr2, laser_sens->corr, laser_sens->nrays);
			cr_ld_draw_corr(cr, laser_ref, laser_sens, &(p->corr));

			cr_set_reference(cr, x_old);
			cr_ld_draw(cr, laser_sens, &(p->laser_sens_s));

		cairo_restore(cr);

		cairo_save(cr);
			cairo_identity_matrix(cr);
			cairo_set_font_size (cr, 12.0);
			cairo_select_font_face (cr, "Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

			char text[100];
			sprintf(text, "Iteration #%d: %s", it, friendly_pose(x_old));
			cairo_move_to(cr,  0.5*width_points, 0.5*height_points );
			cairo_show_text(cr,text );
		cairo_restore(cr);

		cairo_show_page (cr);
	}
	
	ld_free(laser_ref);
	ld_free(laser_sens);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
	
	return 1;
}

