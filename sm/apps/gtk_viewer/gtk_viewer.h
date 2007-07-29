#ifndef H_GTK_VIEWER
#define H_GTK_VIEWER

#include <csm/csm_all.h>
#include <csm/laser_data_cairo.h>

struct GooLaserData;

typedef struct {
	ld_style laser;
	line_style pose_path;

	const char *use;
	ld_reference use_reference;
	const char * input_filename;

	oriented_bbox viewport;
	
	LDP* scans;
	struct GooLaserData** scans_items;
	int scans_size; 
	int scans_num;
	
	double device_size[2];
	
	GooCanvasItem *root;
	GooCanvas*canvas;
} viewer_params;

void compute_transformations(viewer_params*p);

#endif
