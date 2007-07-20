#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#include "../csm/csm_all.h"
#include "../csm/laser_data_drawing.h"

#include <options/options.h>
#include <cairo-pdf.h>

#include "ld_cairo.h"

typedef struct {
	const char * file_input;
	const char * file_output;
	
	ld_style laser_ref_s, laser_sens_s;
	/* Drawing style for correspondences */
	line_style corr;
	
	
	double max_width_cm;
	double max_height_cm;
	
} anim_params ;

int draw_animation( anim_params* p, JO jo);


void set_defaults(anim_params *p) {
	lds_set_defaults(&(p->laser_ref_s));
	lds_set_defaults(&(p->laser_sens_s));
	ls_set_defaults(&(p->corr));
}

int main(int argc, const char** argv)
{
	sm_set_program_name(basename(argv[0]));
	
	anim_params p;
	set_defaults(&p);
	
	struct option* ops = options_allocate(60);
	options_string(ops, "in", &p.file_input, "stdin", "Input file (defaults to stdin)");
	options_string(ops, "out", &p.file_output, "sm_animate.pdf", "Output file ");

	lds_add_options(&(p.laser_ref_s), ops, "ref_", "");
	lds_add_options(&(p.laser_sens_s), ops, "sens_", "");
	ls_add_options(&(p.corr), ops, "corr_", "");
	
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
	
	ld_compute_orientation(laser_ref, 5, 0.004);
	ld_compute_orientation(laser_sens, 5, 0.004);

	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);

	double max_readings = 0;
	int i; for(i=0;i<laser_ref->nrays;i++) {
		if(!ld_valid_ray(laser_ref,i)) continue;
		if(max_readings < laser_ref->readings[i])
			max_readings = laser_ref->readings[i];
	}
	sm_debug("max_readings: %f\n", max_readings);

	double ld_min[2], ld_max[2];
	double pose[3] = {0,0,0};
	if(!ld_get_bounding_box(laser_ref, ld_min, ld_max, pose, p->laser_ref_s.horizon)){
		sm_error("Not enough good points to establish bounding box.\n");
		return 0;
	}
	

	double padding = 0.2;
	ld_min[0] -= padding;
	ld_min[1] -= padding;
	ld_max[0] += padding;
	ld_max[1] += padding;
	
	sm_info("Bounding box: %f %f -- %f %f\n", ld_min[0], ld_min[1], ld_max[0], ld_max[1]);

	int max_width_points = 500;
	int max_height_points = 500;	
	cairo_surface_t *surface;
	cairo_t *cr;
	
	if(!create_pdf_surface(p->file_output, max_width_points, max_height_points, 
		ld_min, ld_max, &surface, &cr)) return 0;

	cairo_set_source_rgb (cr, 0.0, 1.0, 0.2);
	cairo_set_line_width(cr, 0.02);
	cairo_move_to(cr, ld_min[0], ld_min[1]);
	cairo_line_to(cr, ld_min[0], ld_max[1]);
	cairo_line_to(cr, ld_max[0], ld_max[1]);
	cairo_line_to(cr, ld_max[0], ld_min[1]);
	cairo_line_to(cr, ld_min[0], ld_min[1]);
	cairo_stroke(cr);
	
	
	
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
/*			cairo_identity_matrix(cr);*/
			cairo_set_font_size (cr, 0.1);
			cairo_select_font_face (cr, "Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

			char text[100];
			sprintf(text, "Iteration #%d: %s", it, friendly_pose(x_old));
			cairo_move_to(cr,  0.0, 0.0 );
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

