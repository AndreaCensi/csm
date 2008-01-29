/* This algorithm was created by Cyrill Stachniss 
	http://www.informatik.uni-freiburg.de/~stachnis/ */
#include "laser_data_drawing.h"
#include "math_utils.h"
#include "logging.h"

// the 2d-point structure for the input
typedef struct {
  double x;
  double y;
} BB_Point;

// computes the area of minimal bounding box for a set of 2d-points
double getBoundingBoxArea(BB_Point* p, int nOfPoints);

// computes the minimal bounding box for a set of 2d-points
// ul = upper left  point, ll = lower left point, 
// ur = upper right point, ul = upper left point
int getBoundingBox(BB_Point* p, int nOfPoints, 
	double ul[2], double ur[2], double ll[2], double lr[2]);

struct bbfind_imp {
	int num;
	
	int buf_size;
	BB_Point * buf;
};

/* Initialize structure */
bbfind * bbfind_new(void);

/* -------------------------------------- */

bbfind * bbfind_new() {
	bbfind * bbf = malloc(sizeof(bbfind));
	bbf->buf_size = 1000;
	bbf->buf = malloc(sizeof(BB_Point)*bbf->buf_size);
	bbf->num = 0;
	return bbf;
}

int bbfind_add_point(bbfind*bbf, double point[2]) {
	return bbfind_add_point2(bbf, point[0], point[1]);
}

int bbfind_add_point2(bbfind*bbf, double x, double y) {
	if(bbf->num > bbf->buf_size - 2) {
		bbf->buf_size	*= 2;
		if(! (bbf->buf = (BB_Point*) realloc(bbf->buf, sizeof(BB_Point)*bbf->buf_size)) ) {
			sm_error("Cannot allocate (size=%d)\n", bbf->buf_size);
			return 0;
		}
	}
	bbf->buf[bbf->num].x = x;
	bbf->buf[bbf->num].y = y;
	bbf->num++;
	return 1;
}

void oriented_bbox_compute_corners(const BB2 obbox,
	double ul[2], double ur[2], double ll[2], double lr[2]) {
	
	ll[0] = obbox->pose[0];
	ll[1] = obbox->pose[1];
	lr[0] = obbox->pose[0] + obbox->size[0] * cos(obbox->pose[2]);
	lr[1] = obbox->pose[1] + obbox->size[0] * sin(obbox->pose[2]);
	ul[0] = obbox->pose[0] + obbox->size[1] * cos(obbox->pose[2] + M_PI/2);
	ul[1] = obbox->pose[1] + obbox->size[1] * sin(obbox->pose[2] + M_PI/2);
	ur[0] = ul[0] + obbox->size[0] * cos(obbox->pose[2]);
	ur[1] = ul[1] + obbox->size[0] * sin(obbox->pose[2]);
		
}

int bbfind_add_bbox(bbfind*bbf, const BB2 bbox) {
	double ul[2], ur[2], ll[2], lr[2];
	oriented_bbox_compute_corners(bbox, ul, ur, ll, lr);
	return
		bbfind_add_point(bbf, ul) &&
		bbfind_add_point(bbf, ur) &&
		bbfind_add_point(bbf, ll) &&
		bbfind_add_point(bbf, lr);
}


int bbfind_compute(bbfind*bbf, BB2 bbox) {
	double ul[2], ur[2], ll[2], lr[2];
	
	if(1) {
		if(!getBoundingBox(bbf->buf, bbf->num, ul, ur, ll, lr)) {
			sm_error("Could not compute bounding box.\n");
			return 0;
		}
		bbox->pose[0] = ll[0];
		bbox->pose[1] = ll[1];
		bbox->pose[2] = atan2(lr[1]-ll[1], lr[0]-ll[0]);
		bbox->size[0] = distance_d(lr, ll);
		bbox->size[1] = distance_d(ll, ul);
	} else {
		double bb_min[2] = {bbf->buf[0].x,bbf->buf[0].y}, 
				bb_max[2] = {bbf->buf[0].x,bbf->buf[0].y};
		int i; for(i=0;i<bbf->num; i++) {
			bb_min[0] = GSL_MIN(bb_min[0], bbf->buf[i].x);
			bb_min[1] = GSL_MIN(bb_min[1], bbf->buf[i].y);
			bb_max[0] = GSL_MAX(bb_max[0], bbf->buf[i].x);
			bb_max[1] = GSL_MAX(bb_max[1], bbf->buf[i].y);
		}
		bbox->pose[0] = bb_min[0];
		bbox->pose[1] = bb_min[1];
		bbox->pose[2] = 0;
		bbox->size[0] = bb_max[0] - bb_min[0];
		bbox->size[1] = bb_max[1] - bb_min[1];
	}
	return 1;
}

void bbfind_free(bbfind* bbf) {
	free(bbf->buf);
	free(bbf);
}

void ld_get_oriented_bbox(LDP ld, double horizon, oriented_bbox*obbox) {
	bbfind * bbf = bbfind_new();
	int i; for(i=0;i<ld->nrays;i++) {
		if(!ld->valid[i]) continue;
		if(ld->readings[i]>horizon) continue;

		double p0[2] = {
			cos(ld->theta[i]) * ld->readings[i],
			sin(ld->theta[i]) * ld->readings[i]
		};
		
		bbfind_add_point(bbf, p0);
	}
	bbfind_compute(bbf, obbox);
	bbfind_free(bbf);
}

// computes the area of minimal bounding box for a set of 2d-points
double getBoundingBoxArea(BB_Point* p, int nOfPoints) {
	double ul[2], ur[2], ll[2], lr[2];
	
	int wasOk = getBoundingBox(p, nOfPoints, ul, ur, ll, lr);
	double vol = (!wasOk) ? -1.0 : distance_d(ul,ll)*distance_d(ul,ur);
  return vol;
}

// computes the minimal bounding box for a set of 2d-points
// ul = upper left  point, ll = lower left point, 
// ur = upper right point, ul = upper left point
int getBoundingBox(BB_Point* p, int nOfPoints, 
	double ul[2], double ur[2], double ll[2], double lr[2]) {

  // calculate the center of all points (schwerpunkt)
  // -------------------------------------------------
  double centerx = 0;
  double centery = 0;
  for (int i=0; i < nOfPoints; i++) {
	 centerx += p[i].x;
	 centery += p[i].y;
  }	 
  centerx /= (double) nOfPoints;
  centery /= (double) nOfPoints;


  
  // calcutae the covariance matrix
  // -------------------------------
  // covariance matrix (x1 x2, x3 x4) 
  double x1 = 0.0;
  double x2 = 0.0;
  double x3 = 0.0;
  double x4 = 0.0;

  for (int i=0; i < nOfPoints; i++) {
	 double cix = p[i].x - centerx;
	 double ciy = p[i].y - centery;
	 
	 x1 += cix*cix;
	 x2 += cix*ciy;  
	 x4 += ciy*ciy;
  }
  x1 /= (double) nOfPoints;
  x2 /= (double) nOfPoints;
  x3 = x2;
  x4 /= (double) nOfPoints;
  // covariance & center  done


  // calculate the eigenvectors
  // ---------------------------
  // catch 1/0 or sqrt(<0)
  if ((x3 == 0) || (x2 == 0)|| (x4*x4-2*x1*x4+x1*x1+4*x2*x3 < 0 ))  {
	sm_error("Cyrill: Could not compute bounding box.\n");
	return 0;
}

 // eigenvalues
  double lamda1 = 0.5* (x4 + x1 + sqrt(x4*x4 - 2.0*x1*x4 + x1*x1 + 4.0*x2*x3));
  double lamda2 = 0.5* (x4 + x1 - sqrt(x4*x4 - 2.0*x1*x4 + x1*x1 + 4.0*x2*x3));
  
  // eigenvector 1  with  (x,y)
  double v1x = - (x4-lamda1) * (x4-lamda1) * (x1-lamda1) / (x2 * x3 * x3);
  double v1y = (x4-lamda1) * (x1-lamda1) / (x2 * x3);
  // eigenvector 2 with	 (x,y)
  double v2x = - (x4-lamda2) * (x4-lamda2) * (x1-lamda2) / (x2 * x3 * x3);
  double v2y = (x4-lamda2) * (x1-lamda2) / (x2 * x3);

  // norm the eigenvectors
  double lv1 = sqrt ( (v1x*v1x) + (v1y*v1y) );
  double lv2 = sqrt ( (v2x*v2x) + (v2y*v2y) );
  v1x /= lv1;
  v1y /= lv1;
  v2x /= lv2;
  v2y /= lv2;
  // eigenvectors done

  // get the points with maximal dot-product 
  double x = 0.0;
  double y = 0.0;
  double xmin = 1e20;
  double xmax = -1e20;
  double ymin = 1e20;
  double ymax = -1e20;
  for(int i = 0; i< nOfPoints; i++) {
	 // dot-product of relativ coordinates of every point
	 x = (p[i].x - centerx) * v1x +	(p[i].y - centery) * v1y;
	 y = (p[i].x - centerx) * v2x +	(p[i].y - centery) * v2y;

	 if( x > xmax) xmax = x;
	 if( x < xmin) xmin = x;
	 if( y > ymax) ymax = y;
	 if( y < ymin) ymin = y;
  }

  // now we can compute the corners of the bounding box
	if(ul) {
		ul[0] = centerx + xmin * v1x + ymin * v2x;
		ul[1] = centery + xmin * v1y + ymin * v2y;
	}

	if(ur) {
		ur[0] = centerx + xmax * v1x + ymin * v2x;
		ur[1] = centery + xmax * v1y + ymin * v2y;
	}
	
	if(ll) {
		ll[0] = centerx + xmin * v1x + ymax * v2x;
		ll[1] = centery + xmin * v1y + ymax * v2y;
	}
	
	if(lr) {
		lr[0] = centerx + xmax * v1x + ymax * v2x;
		lr[1] = centery + xmax * v1y + ymax * v2y;
	}
	return 1;
}




