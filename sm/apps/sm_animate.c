#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cairo-pdf.h>

#include <options/options.h>

#include "../csm/csm_all.h"
#include "../csm/laser_data_drawing.h"
#include "../csm/laser_data_cairo.h"

typedef struct {
	const char * file_input;

	const char * file_output;
	
	ld_style laser_ref_s, laser_sens_s;
	/* Drawing style for correspondences */
	line_style corr;
	
	
	double max_width_cm;
	double max_height_cm;
	
	int max_iterations;
	int zoom_ray;
	
	/* maximum size, in points, of the output pdf */
	int width_pt, height_pt;
	
	/* Padding, in meters, to be added */
	double padding;
	
	int write_info;
} anim_params ;

int draw_animation( anim_params* p, JO jo, const char*filename);
void set_defaults(anim_params *p);


int main(int argc, const char** argv)
{
	sm_set_program_name(argv[0]);

	anim_params p;
	set_defaults(&p);
	
	struct option* ops = options_allocate(100);
	options_string(ops, "in", &p.file_input, "stdin", "Input file (defaults to stdin)");
	options_string(ops, "out", &p.file_output, "sm_animate_%02d.pdf", "Output file ");

	options_int(ops, "write_info", &p.write_info, 0, "Writes informations and statistics in the picture.");
	options_int(ops, "max_iterations", &p.max_iterations, 10, "Maximum number of iterations");
	options_int(ops, "zoom_ray", &p.zoom_ray, -1, "If >= 0, the action is zoomed on a particular ray.");
	options_int(ops, "width_pt", &p.width_pt, 500, "Maximum width, in points, of the PDF.");
	options_int(ops, "height_pt", &p.height_pt, 500, "Maximum height, in points, of the PDF.");
	options_double(ops, "padding", &p.padding, 0.2, "Padding, in meters, to be added around figure.");

	lds_add_options(&(p.laser_ref_s), ops, "ref_", "");
	lds_add_options(&(p.laser_sens_s), ops, "sens_", "");
	ls_add_options(&(p.corr), ops, "corr_", "");
	
	if(!options_parse_args(ops, argc, argv)) {
		sm_info("Draws ICP animation. It reads the output created by sm2 when given the 'file_jj' switch. \n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * input = open_file_for_reading(p.file_input);
	if(!input) return -1;
	
	JO jo; int count = 0;
	while( (jo = json_read_stream(input)) ) {
		char filename[100];
		sprintf(filename, p.file_output, count);
		sm_info("Writing frame %s \n", p.file_output);
		if(!draw_animation(&p, jo, filename))
			return -2;
		count++;
	}

	return 0;
}

/** Returns an array with depths */
int draw_animation(anim_params* p, JO jo, const char*filename) {
	JO jo_ref = jo_get(jo, "laser_ref");
	JO jo_sens = jo_get(jo, "laser_sens");
	if(!jo_ref || !jo_sens) {
		sm_error("Could not get laser_ref/laser_sens.\n");
		return 0;
	}
	
	LDP laser_ref = json_to_ld(jo_ref);
	LDP laser_sens = json_to_ld(jo_sens); 
	if(!laser_ref || !laser_sens) {
		sm_error("Could not read laser_ref/laser_sens from JSON representation.\n");
		return 0;
	}
	
	ld_compute_cartesian(laser_ref);
	ld_compute_cartesian(laser_sens);

	double ld_min[2], ld_max[2];
	if(p->zoom_ray == -1) {
		double zero[3] = {0,0,0};
		if(!ld_get_bounding_box(laser_ref, ld_min, ld_max, zero, p->laser_ref_s.horizon)){
			sm_error("Not enough good points to establish bounding box.\n");
			return 0;
		}
	} else {
		if(p->zoom_ray < 0 || p->zoom_ray >= laser_ref->nrays || !ld_valid_ray(laser_ref, p->zoom_ray)) {
			sm_error("Ray index #%d is not valid in laser_ref.\n", p->zoom_ray);
			return 0;
		}
		
		ld_min[0] = ld_max[0] = laser_ref->points[p->zoom_ray].p[0];
		ld_min[1] = ld_max[1] = laser_ref->points[p->zoom_ray].p[1];
	}
	
	ld_min[0] -= p->padding;
	ld_min[1] -= p->padding;
	ld_max[0] += p->padding;
	ld_max[1] += p->padding;
	
	sm_info("Bounding box: %f %f -- %f %f\n", ld_min[0], ld_min[1], ld_max[0], ld_max[1]);

	cairo_surface_t *surface;
	cairo_t *cr;
	
	if(!create_pdf_surface(filename, p->width_pt, p->height_pt, 
		ld_min, ld_max, &surface, &cr)) return 0;
	
	JO iterations = jo_get(jo, "iterations");
	if(!iterations || !json_object_is_type(iterations, json_type_array)) {
		fprintf(stderr, "Could not read iterations.\n");
		return 0;
	}
	
	int niterations = json_object_array_length(iterations);
	if(niterations>p->max_iterations) niterations = p->max_iterations;
	sm_info("Displaying %d iterations.\n", niterations);

	int it;
	for(it=0;it<niterations;it++) {
		JO iteration = json_object_array_get_idx(iterations, it);
		
		double x_old[3], x_new[3];
		jo_read_double_array(iteration, "x_old", x_old, 3, NAN);
		jo_read_double_array(iteration, "x_new", x_new, 3, NAN);


		cairo_save(cr);
			cr_ld_draw(cr, laser_ref, &(p->laser_ref_s));

			ld_compute_world_coords(laser_sens, x_old);


			JO corr0 = jo_get(iteration, "corr0");
			JO corr1 = jo_get(iteration, "corr1");
			JO corr2 = jo_get(iteration, "corr2");
			if(!corr1 || !corr2 || !corr0) {
				sm_error("Iteration %d: could not read correspondences (field 'corr<i>'). Probably ICP failed here?\n", it);
			} else {
				cr_set_style(cr, &(p->corr));
				cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
				json_to_corr(corr0, laser_sens->corr, laser_sens->nrays);
				cr_ld_draw_corr(cr, laser_ref, laser_sens);

				cairo_set_source_rgb (cr, 1.0, 0.0, 1.0);
				json_to_corr(corr1, laser_sens->corr, laser_sens->nrays);
				cr_ld_draw_corr(cr, laser_ref, laser_sens);

				cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
				json_to_corr(corr2, laser_sens->corr, laser_sens->nrays);
				cr_ld_draw_corr(cr, laser_ref, laser_sens);
			}

			cr_set_reference(cr, x_old);
			cr_ld_draw(cr, laser_sens, &(p->laser_sens_s));

		cairo_restore(cr);

		if(p->write_info) {
			cairo_save(cr);
				cairo_identity_matrix(cr);
				cairo_set_font_size (cr, 20.0f);
				cairo_select_font_face (cr, "Sans",
				    CAIRO_FONT_SLANT_NORMAL,
				    CAIRO_FONT_WEIGHT_NORMAL);

				char text[100];
				sprintf(text, "Iteration #%d: x_old: %s", it, friendly_pose(x_old));
				cairo_move_to(cr,  0.0, -20.0 );
				cairo_show_text(cr, text);
				
				sm_info("%s\n",text);
			cairo_restore(cr);
		}
	
		cairo_show_page (cr);
	}
	
	ld_free(laser_ref);
	ld_free(laser_sens);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
	
	return 1;
}


void set_defaults(anim_params *p) {
	lds_set_defaults(&(p->laser_ref_s));
	lds_set_defaults(&(p->laser_sens_s));
	ls_set_defaults(&(p->corr));
	p->laser_ref_s.points.color = "#00f";
	p->laser_sens_s.points.color = "#f00";
	p->laser_ref_s.pose.color = p->laser_ref_s.points.color;
	p->laser_sens_s.pose.color = p->laser_sens_s.points.color;
	p->laser_sens_s.pose_radius = 
	p->laser_ref_s.pose_radius = 0.015;
	
}

