#include <stdlib.h>
#include <goocanvas.h>
#include "goo_laser_data.h"
#include "gtk_viewer.h"

static gboolean on_rect_button_press (GooCanvasItem  *view,
				      GooCanvasItem  *target,
				      GdkEventButton *event,
				      gpointer        data);

static gboolean on_delete_event      (GtkWidget      *window,
				      GdkEvent       *event,
				      gpointer        unused_data);

void* reading_thread(void *data);
	
GooCanvasItem*  text_item;
GooCanvasItem* rect_item;

int main (int argc, char **argv)
{
	sm_set_program_name(basename(argv[0]));
	
	viewer_params *p = (viewer_params*) malloc(sizeof(viewer_params));
	lds_set_defaults(&(p->laser));
	ls_set_defaults(&(p->pose_path));
	
	p->laser.rays.draw = 0;
	p->laser.points.draw = 0;
	p->laser.normals.draw = 0;
	p->laser.countour.width = 0.1;
	p->pose_path.width = 0.1;
	p->pose_path.color = "#f00";
	
	struct option * ops = options_allocate(100);
	options_string(ops, "in", &(p->input_filename), "stdin", "input file (Carmen or JSON)");
	options_string(ops, "use", &(p->use), "estimate", "One in 'odometry','estimate','true_pose'");
	
	lds_add_options(&(p->laser), ops, "laser_", "");
	ls_add_options(&(p->pose_path), ops, "path_", "");

	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple experimental GTK viewer.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

  /* init threads */
  g_thread_init(NULL);
  gdk_threads_init();


  GtkWidget *window, *scrolled_win, *canvas;
GooCanvasItem *root;

  /* Initialize GTK+. */
  gtk_set_locale ();
  gtk_init (&argc, &argv);

  /* Create the window and widgets. */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 600);
  gtk_widget_show (window);
  g_signal_connect (window, "delete_event", (GtkSignalFunc) on_delete_event,
		    NULL);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
				       GTK_SHADOW_IN);
  gtk_widget_show (scrolled_win);
  gtk_container_add (GTK_CONTAINER (window), scrolled_win);

  canvas = goo_canvas_new ();

  root = goo_canvas_get_root_item (GOO_CANVAS (canvas));

	p->device_size[0] = 800;
	p->device_size[1] = 600;
	
	gtk_widget_set_size_request (canvas, p->device_size[0], p->device_size[1]);
	goo_canvas_set_bounds (GOO_CANVAS (canvas), 0, 0, p->device_size[0], p->device_size[1]);
	gtk_widget_show (canvas);
	gtk_container_add (GTK_CONTAINER (scrolled_win), canvas);

	p->root = root;
	p->canvas = canvas;
	
	
	/* Add a few simple items. */
rect_item = goo_canvas_rect_new (root, 0, 0, 50, 50,
				   "line-width", 10.0,
				   "radius-x", 20.0,
				   "radius-y", 10.0,
				   "stroke-color", "yellow",
				   "fill-color", "red",
				   NULL);

/*text_item = goo_canvas_text_new (root, "Hello World", 300, 300, -1,
				   GTK_ANCHOR_CENTER,
				   "font", "Sans 1",
				   NULL);
  goo_canvas_item_rotate (text_item, 45, 300, 300);
  */

	
	GError * error;
	if (!g_thread_create(reading_thread, p, FALSE, &error)) {
      g_printerr ("Failed to create YES thread: %s\n", error->message);
      return 1;
	}
  /* Pass control to the GTK+ main event loop. */
  gtk_main ();

}

void* reading_thread(void *data) {
	viewer_params * p = (viewer_params*) data;
	
	FILE * input = open_file_for_reading(p->input_filename);
	if(!input) return 0;
	
	p->scans = malloc(1);
	p->scans_items = malloc(1);
	p->scans_size = 0;
	p->scans_num = 0;
	
	LDP ld;
	while( (ld = ld_read_smart(input)) ) {
	   
		if(p->scans_num >= p->scans_size) {
			p->scans_size = 2* p->scans_size +10;
			p->scans = realloc(p->scans, sizeof(LDP) * p->scans_size);
			p->scans_items = realloc(p->scans_items, sizeof(LDP) * p->scans_size);
		}
		
		gdk_threads_enter();

		if(1) {
			GooCanvasItem * gld = goo_laser_data_new (p->root, p, ld);
		g_signal_connect (gld, "button_press_event",
			    (GtkSignalFunc) on_rect_button_press, NULL);
			p->scans[p->scans_num] = ld;
			p->scans_items[p->scans_num] = gld;
			p->scans_num++;
		} else {

		}
		compute_transformations(p);
		goo_canvas_update(p->canvas);
/*		gdk_flush ();*/
		
		gdk_threads_leave();
		/* sleep a while */
		usleep(20);
/*	      sleep(g_random_int_range (1, 4));*/
	}

  return 0;
}

void world_to_viewport(
	const oriented_bbox*obbox, const double device_size[2], cairo_matrix_t*m) {
	
	cairo_matrix_init_identity(m);
	
	double scale[2] = { 
		device_size[0] / obbox->size[0], 
		device_size[1] / obbox->size[1]};
		
	double s = scale[0] < scale[1] ? scale[0] : scale[1];
	cairo_matrix_scale(m, s, s);
	cairo_matrix_scale(m, 1, -1);
	cairo_matrix_rotate(m, -obbox->pose[2]);	
	cairo_matrix_translate(m, -obbox->pose[0], -obbox->pose[1]);
	
}

void item_to_world(const double pose[3], cairo_matrix_t*m) {
	cairo_matrix_init_identity(m);
	cairo_matrix_translate(m, pose[0], pose[1]);
	cairo_matrix_rotate(m, pose[2]);	
}

void compute_transformations(viewer_params*p) {
	int k;
	oriented_bbox global;
	
	if(0) {
		bbfind * bbf = bbfind_new();
		int n = p->scans_num;
/*		if(n>20) n = 20;*/
	
		for(k=0;k<n;k++) {
			GooLaserData * gld = (GooLaserData*) p->scans_items[k];
			bbfind_add_bbox(bbf, &gld->obbox);
		}
	
	
		if(bbfind_compute(bbf, &global)) {
			sm_debug("%d Global: %s size %f %f\n", p->scans_num, friendly_pose(global.pose), 
				global.size[0], global.size[1]);
		} else {
			sm_error("%d Could not compute global bounding box.\n", p->scans_num);
		}
		bbfind_free(bbf);
	}else{
	
	global.pose[0] = -22;
	global.pose[1] = -41;
	global.pose[2] = M_PI/2;
	global.size[0] = 106;
	global.size[1] = 46;
}
	cairo_matrix_t m_world_to_viewport;
	world_to_viewport(&global, p->device_size, &(m_world_to_viewport));

	cairo_matrix_t *m = &m_world_to_viewport;
	sm_info("Matrix: %f %f %f %f  %f %f\n", m->xx,m->yx,m->xy,m->yy, m->x0, m->y0);

	cairo_matrix_t mm;
	cairo_matrix_init_identity(&mm);
/*	cairo_matrix_translate(&mm, 300, 600);*/
	cairo_matrix_translate(&mm, 300, 600);
	cairo_matrix_rotate(&mm, deg2rad(p->scans_num));
	goo_canvas_item_set_transform(rect_item, &mm);

	/*
	goo_canvas_item_set_transform(text_item, &m_world_to_viewport);
*/
	for(k=0;k<p->scans_num;k++) {
		GooCanvasItem * gld = p->scans_items[k];
		
		double * pose = p->scans[k]->estimate;
		cairo_matrix_t m_item_to_world;
		item_to_world(pose, &m_item_to_world);

		cairo_matrix_t transform;
		cairo_matrix_multiply(&transform, &m_item_to_world, &m_world_to_viewport);
		goo_canvas_item_set_transform(gld, &transform);
	}
}

/* This handles button presses in item views. We simply output a message to
   the console. */
static gboolean
on_rect_button_press (GooCanvasItem  *item,
		      GooCanvasItem  *target,
		      GdkEventButton *event,
		      gpointer        data)
{
  g_print ("rect item received button press event\n");
  return TRUE;
}


/* This is our handler for the "delete-event" signal of the window, which
   is emitted when the 'x' close button is clicked. We just exit here. */
static gboolean
on_delete_event (GtkWidget *window,
		 GdkEvent  *event,
		 gpointer   unused_data)
{
  exit (0);
}
