#include <time.h>
#include <string.h>
#include <libgen.h>

#include <cairo.h>
#include <cairo-pdf.h>

#include <options/options.h>

#include <csm/csm_all.h>
#include <csm/laser_data_drawing.h>
#include <csm/laser_data_cairo.h>

typedef struct {
	int interval;
	const char*use;
	double padding;
	double dimension;
	
	int draw_confidence;
	double confidence_mult;
	
	const char*output_filename;
	const char*input_filename;

	ld_reference use_reference;

	double offset_theta_deg;
	
	/* Drawing style for scans */
	ld_style laser;
	/* Drawing style for robot path */
	line_style pose_path;
	
} log2pdf_params;

int log2pdf(log2pdf_params *p);

double offset_theta = 0;

int main(int argc, const char*argv[]) {
	sm_set_program_name(basename(argv[0]));

	log2pdf_params p;
	
	lds_set_defaults(&(p.laser));
	ls_set_defaults(&(p.pose_path));
	
	p.laser.rays.draw = 0;
	p.laser.points.draw = 0;
	p.laser.normals.draw = 0;
	p.laser.countour.width = 0.1;
	p.pose_path.width = 0.1;
	p.pose_path.color = "#f00";
	
	struct option * ops = options_allocate(100);
	options_int(ops, "interval", &p.interval, 10, "how many to ignore");
	options_string(ops, "in", &p.input_filename, "stdin", "input file (Carmen or JSON)");
	options_string(ops, "out", &p.output_filename, "", "output file (if empty, input file + '.pdf')");
	options_double(ops, "padding", &p.padding, 0.2, "padding around bounding box (m)");
	options_double(ops, "dimension", &p.dimension, 500.0, "dimension of the image (points)");
	options_int(ops, "draw_confidence", &p.draw_confidence, 0, " Draws confidence (cov_readings[i]) ");
	options_double(ops, "confidence_mult", &p.confidence_mult, 3.0, " 3-sigma ");
	options_double(ops, "offset_theta_deg", &p.offset_theta_deg, 0.0, " rotate entire map by this angle (deg) ");

	options_string(ops, "use", &p.use, "estimate", "One in 'odometry','estimate','true_pose'");
	
	lds_add_options(&(p.laser), ops, "laser_", "");
	ls_add_options(&(p.pose_path), ops, "path_", "");
	
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
		if(!strcmp(p.use, ld_reference_to_string(i) ))
			p.use_reference = (ld_reference) i;

	if(Invalid == p.use_reference) {
		sm_error("Invalid reference '%s'. " 
			"Use one in 'odometry','estimate','true_pose'.\n", p.use);
		return -1;
	}
	
	
	log2pdf(&p);
	return 0;
}


int log2pdf(log2pdf_params *p) {

	/** First of all, we read the entire map into memory */
	FILE *input_file = open_file_for_reading(p->input_filename);
	if(!input_file) return 0;
	
	LDP*scans; int nscans;
	if(!ld_read_some_scans(input_file, &scans, &nscans, p->interval)) {
		sm_error("Could not read map.\n"); 
		return 0;
	}
	
	sm_info("Read map: %d scans in total.\n", nscans);

	/** Let's find the bounding box for the map */
	double bb_min[2], bb_max[2];
	double offset[3] = {0,0,0};
	lda_get_bounding_box(scans, nscans, bb_min, bb_max, offset, Estimate, p->laser.horizon);
	
	bb_min[0] -= p->padding;
	bb_min[1] -= p->padding;
	bb_max[0] += p->padding;
	bb_max[1] += p->padding;
	

	sm_info("Bounding box: %f %f -- %f %f.\n", bb_min[0], bb_min[1],
		bb_max[0], bb_max[1]);
		
	/* Create PDF surface and setup paper size and transformations */
	int max_width_points = p->dimension;
	int max_height_points = p->dimension;
	cairo_surface_t *surface;
	cairo_t *cr;

	if(!create_pdf_surface(p->output_filename, max_width_points, max_height_points, 
		bb_min, bb_max, &surface, &cr)) return 0;

	/* Draw pose path */
	if(p->pose_path.draw) {
		cairo_save(cr);
		cr_set_style(cr, &(p->pose_path));
		cr_lda_draw_pose_path(cr, scans, nscans, p->use_reference);
		cairo_restore(cr);
	}

	/* Draw map */
	int k; for(k=0;k<nscans;k++) {
		LDP ld = scans[k];
		double *pose = ld_get_reference_pose(ld, p->use_reference);
		if(!pose) continue;
		
		double offset[3] = {0,0, deg2rad(p->offset_theta_deg) };
		double world_pose[3];
		oplus_d(offset, pose, world_pose);
				
		cairo_save(cr);
		cr_set_reference(cr, world_pose);
		cr_ld_draw(cr, ld, &(p->laser));
		cairo_restore(cr);
	}

	cairo_show_page (cr);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
	return 1;
}




