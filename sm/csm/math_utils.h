#ifndef H_MATH_UTILS
#define H_MATH_UTILS


/* Sometimes I really don't understand compilers.. */ 
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#ifndef NAN
#define NAN GSL_NAN
#endif
     
void transform_d(const double point2d[2], const double pose[3], double result2d[2]);

/** Returns norm of 2D point p */
double norm_d(const double*p);

double distance_squared_d(const double *a, const double *b);
double distance_d(const double *a, const double *b);


double angleDiff(double a, double b);
double square(double x);
double deg2rad(double deg);
double rad2deg(double rad);


int minmax(int from,int to,int x);

/** Copies n doubles from from to to */
void copy_d(const double*from, int n, double*to);

/* With doubles */
void ominus_d(const double x[3], double res[3]);
void oplus_d(const double x1[3], const double x2[3], double res[3]);
void pose_diff_d(const double second[3], const double first[3], double res[3]);
	
/** Projects (p[0],p[1]) on the LINE passing through (ax,ay)-(bx,by). If distance!=0, distance is set
to the distance from the point to the segment */
void projection_on_line_d(const double *a,
	const double *b,
	const double *p,
	double *res,
	double *distance);
	
/** Distance of x from its projection on segment a-b */
double dist_to_segment_d(const double*a, const double*b, const double*x);
/** Same thing as dist_to_segment_d(), but squared */
double dist_to_segment_squared_d(const double*a, const double*b, const double*x);

void projection_on_segment_d(
	const double*a,
	const double*b,
	const double*x,
	      double*proj);

/** A function to print poses and covariances in a friendly way */
const char* friendly_pose(double*pose);

/** Returns true v is NAN */
int is_nan(double v);

/** Returns true if any value in d is NAN */
int any_nan(const double *d, int n);

/** Count numbers of items in array v equal to value */
int count_equal(const int*v, int n, int value);


#endif

