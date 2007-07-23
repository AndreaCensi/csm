#ifndef H_GOO_LASER_DATA
#define H_GOO_LASER_DATA

#include <gtk/gtk.h>
#include "goocanvasitemsimple.h"

#include <csm/csm.h>

#include "gtk_viewer.h"

G_BEGIN_DECLS

typedef struct _GooDemoItem       GooDemoItem;
typedef struct _GooDemoItemClass  GooDemoItemClass;

typedef struct {
	GooCanvasItemSimple parent_object;

	double bb_min[2], bb_max[2];
	oriented_bbox obbox;

	LDP ld;
	
	viewer_params *p;
} GooLaserData;

typedef struct {
  GooCanvasItemSimpleClass parent_class;
} GooLaserDataClass;

GType               goo_laser_data_get_type  (void) G_GNUC_CONST;
GooCanvasItem*      goo_laser_data_new (GooCanvasItem *parent, viewer_params *p,
LDP ld);

G_END_DECLS

#endif
