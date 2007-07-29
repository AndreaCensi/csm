
#include "goo_laser_data.h"

/* Use the GLib convenience macro to define the type. GooLaserData is the
   class struct, goo_laser_data is the function prefix, and our class is a
   subclass of GOO_TYPE_CANVAS_ITEM_SIMPLE. */
G_DEFINE_TYPE (GooLaserData, goo_laser_data, GOO_TYPE_CANVAS_ITEM_SIMPLE)


/* The standard object initialization function. */
static void
goo_laser_data_init (GooLaserData *gld)
{
	
}


/* The convenience function to create new items. This should start with a 
   parent argument and end with a variable list of object properties to fit
   in with the standard canvas items. */
GooCanvasItem*
goo_laser_data_new (GooCanvasItem      *parent, viewer_params *p, LDP ld) 
{
  GooCanvasItem *item;
  GooLaserData *gld;

	item = g_object_new (goo_laser_data_get_type(), NULL);

	gld = (GooLaserData*) item;

	ld_get_bounding_box(ld, gld->bb_min, gld->bb_max, ld->estimate, 10);
	double padding = 1;
	gld->bb_min[0] -= padding;
	gld->bb_min[1] -= padding;
	gld->bb_max[0] += padding;
	gld->bb_max[1] += padding;
	
	
	gld->p = p;
	gld->ld = ld;

	ld_get_oriented_bbox(ld, 20, &(gld->obbox) );
	oplus_d(ld->estimate, gld->obbox.pose, gld->obbox.pose);

/*  va_start (var_args, height);
  first_property = va_arg (var_args, char*);
  if (first_property)
    g_object_set_valist ((GObject*) item, first_property, var_args);
  va_end (var_args);
*/
  if (parent)
    {
      goo_canvas_item_add_child (parent, item, -1);
      g_object_unref (item);
    }

  return item;
}


/* The update method. This is called when the canvas is initially shown and
   also whenever the object is updated and needs to change its size and/or
   shape. It should calculate its new bounds in its own coordinate space,
   storing them in simple->bounds. */
static void
goo_laser_data_update  (GooCanvasItemSimple *simple,
		       cairo_t             *cr)
{
  GooLaserData *gld = (GooLaserData*) simple;

double padding = 0;
  /* Compute the new bounds. */
  simple->bounds.x1 = gld->bb_min[0] - padding;
  simple->bounds.y1 = gld->bb_min[1] - padding;
  simple->bounds.x2 = gld->bb_max[0] + padding;
  simple->bounds.y2 = gld->bb_max[1] + padding;

/*sm_debug("Bound %f %f %f %f\n", gld->bb_min[0],
gld->bb_min[1],	gld->bb_max[0],
gld->bb_max[1]);*/

}

/* The paint method. This should draw the item on the given cairo_t, using
   the item's own coordinate space. */
static void
goo_laser_data_paint (GooCanvasItemSimple   *simple,
		     cairo_t               *cr,
		     const GooCanvasBounds *bounds)
{
	GooLaserData *gld = (GooLaserData*) simple;
/*
	cairo_set_line_width (cr, 0.01);
	cairo_move_to (cr, gld->bb_min[0], gld->bb_min[1]);
	cairo_line_to (cr, gld->bb_max[0], gld->bb_min[1]);
	cairo_line_to (cr, gld->bb_max[0], gld->bb_max[0]);
	cairo_line_to (cr, gld->bb_min[0], gld->bb_max[0]);
	cairo_line_to (cr, gld->bb_min[0], gld->bb_min[1]);
	cairo_close_path (cr);
	cairo_set_source_rgb(cr, 0.8, 0.9, 0.8);
	cairo_stroke (cr);
*/

	cairo_set_antialias( cr, CAIRO_ANTIALIAS_NONE );


	cairo_set_source_rgb(cr, 0.3, 0, 1.0);
	cairo_arc(cr, 0,0,  0.4, 0.0, 2*M_PI);
	cairo_fill(cr);
	
	cr_ld_draw(cr, gld->ld, &(gld->p->laser));
}


/* Hit detection. This should check if the given coordinate (in the item's
   coordinate space) is within the item. If it is it should return TRUE,
   otherwises it should return FALSE. */
static gboolean
goo_laser_data_is_item_at (GooCanvasItemSimple *simple,
			  gdouble              x,
			  gdouble              y,
			  cairo_t             *cr,
			  gboolean             is_pointer_event)
{
  GooLaserData *gld = (GooLaserData*) simple;

/*  if (x < gld->x || (x > gld->x + gld->width)
      || y < gld->y || (y > gld->y + gld->height))*/
    return FALSE;

  return TRUE;
}


/* The class initialization function. Here we set the class' update(), paint()
   and is_item_at() methods. */
static void
goo_laser_data_class_init (GooLaserDataClass *klass)
{
  GooCanvasItemSimpleClass *simple_class = (GooCanvasItemSimpleClass*) klass;

  simple_class->simple_update        = goo_laser_data_update;
  simple_class->simple_paint         = goo_laser_data_paint;
  simple_class->simple_is_item_at    = goo_laser_data_is_item_at;
}


