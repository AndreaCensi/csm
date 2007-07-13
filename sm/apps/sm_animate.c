
int main() {}
#if 0

#include <stdio.h>
#include <string>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fig/fig.h>
#include <fig/fig_utils.h>

#include <simplemap/simplemap.h>

#include <csm/csm_all.h>

#include <options/options.h>

#include "ld_drawing.h"

using namespace std;
using namespace FIG;
using namespace RDK2::Geometry;
using namespace PPU;
using namespace CSM;

struct anim_params {
	const char * file_input;
	const char * file_output;
	int depth_increment;
	
	ld_style laser_ref, laser_sens;
	Fig_style corr;
};

extern "C" FILE * open_file_for_reading(const char*);
int draw_animation(Figure& fig, struct anim_params& p, JO jo, JO log);

void ld_draw_ld(Figure&fig, LDP ld, ld_style&p, Point2od pose) {
	if(p.rays.draw) 
		ld_draw_rays(fig, ld, &(p.rays), pose);
	if(p.countour.draw) 
		ld_draw_countour(fig, ld, &(p.countour), pose);
	if(p.points.draw) 
		ld_draw_points(fig, ld, &(p.points), pose, p.points_radius);
}

int main(int argc, const char** argv)
{
	struct anim_params p;
	
	struct option* ops = options_allocate(60);
	options_string(ops, "in", &p.file_input, "-", "Input file (defaults to stdin)");
	options_string(ops, "out", &p.file_output, "animation.fig", "Output file ");
	options_int(ops, "depth_increment", &p.depth_increment, 5, "depth increment for each frame");

	p.laser_ref.add_options(ops, "ref","Ref");
	p.laser_sens.add_options(ops, "sens","Sens");
	p.corr.add_options(ops, "corr",     "Correspondences |");
	
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "%s: draws icp animation.\n\nUsage:\n",
			basename(*argv));
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * input = open_file_for_reading(p.file_input);
	if(!input) return -1;
	
	JO jo; 
	int count = 0; 	
	while( (jo = json_read_stream(input)) ) {
		Figure fig;

		JO log = jo_new();
		char * base = basename(p.file_output);
		
		jo_add(log, "fig", jo_new_string(base) );
/*
		JO sm_icp = find_object_with_name(jo, "sm_icp");
		if(!sm_icp) {
			fprintf(stderr, "Could find 'sm_icp'.\n");
			return -1;
		} */
			if(!draw_animation(fig, p, jo, log))
			return 0;
		
		fig.write(p.file_output);
		ofstream ofs( (string(p.file_output) + ".desc").c_str());
		ofs << json_object_to_json_string(log);
		jo_free(log);
		
		jo_free(jo);
		count++;
	}
	
	
	
}

/** Returns an array with depths */
int draw_animation(Figure& fig, struct anim_params& p, JO jo, JO log) {
	JO jo_ref = jo_get(jo, "laser_ref");
	JO jo_sens = jo_get(jo, "laser_sens");
	if(!jo_ref || !jo_sens) {
		fprintf(stderr, "Could not get laser_ref/laser_sens.\n");
		return 0;
	}
	
	LDP laser_ref = json_to_ld(jo_ref);
	LDP laser_sens = json_to_ld(jo_sens); 
	if(!laser_ref || !laser_sens) {
		fprintf(stderr, "Could not read laser_ref/laser_sens.\n");
		return 0;
	}
	
	JO iterations = jo_get(jo, "iterations");
	if(!iterations || !json_object_is_type(iterations, json_type_array)) {
		fprintf(stderr, "Could not read iterations.\n");
		return 0;
	}
	
	int niterations = json_object_array_length(iterations);
	fprintf(stderr, "Reading %d iterations.\n", niterations);
	int it;

	{
		JO frames = jo_new_array();
		for(it=0;it<niterations;it++) {
			JO frame = jo_new();

			int depths[4] = {p.corr.depth, p.laser_sens.rays.depth, 
				p.laser_sens.countour.depth, p.laser_sens.points.depth};
			int delta = p.depth_increment * it;
			int a; for(a=0;a<4;a++) depths[a] += delta;

			jo_add_int_array(frame, "depths", depths, 4);
			jo_array_add(frames, frame);
		}
		jo_add(log, "frames", frames);
		int depths[3] = { p.laser_ref.rays.depth, 
			p.laser_ref.countour.depth, p.laser_ref.points.depth};
		jo_add_int_array(log, "common_depths", depths, 3);
	}
	
	ld_draw_ld(fig, laser_ref, p.laser_ref, Point2od(0,0,0));
	for(it=0;it<niterations;it++) {
		JO iteration = json_object_array_get_idx(iterations, it);
		
		double x_old[3], x_new[3];
		jo_read_double_array(iteration, "x_old", x_old, 3, NAN);
		jo_read_double_array(iteration, "x_new", x_new, 3, NAN);
		JO corr2 = jo_get(iteration, "corr2");
		if(!corr2 || !json_to_corr(corr2, laser_sens->corr, laser_sens->nrays)) {
			fprintf(stderr, "Could not read corr2\n");
			return 0;
		}

		ld_draw_correspondences(fig, laser_ref, laser_sens, &(p.corr), Point2od(x_old));
		ld_draw_ld(fig, laser_sens, p.laser_sens, Point2od(x_old));
		
		p.corr.depth                += p.depth_increment;
		p.laser_sens.rays.depth     += p.depth_increment;
		p.laser_sens.countour.depth += p.depth_increment;
		p.laser_sens.points.depth   += p.depth_increment;
	}
	
	ld_free(laser_ref);
	ld_free(laser_sens);
	
	return 1;
}

#endif


