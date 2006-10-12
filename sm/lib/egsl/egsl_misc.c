#include <math.h>
#include "egsl.h"

val egsl_rot(double theta) {
	double R[2*2] = {
		cos(theta), -sin(theta),
		sin(theta),  cos(theta)
	};
	return egsl_vFda(2,2,R);
}

val egsl_zeros(size_t rows, size_t columns) {
	val v = egsl_alloc(rows,columns);
	gsl_matrix * m = egsl_gslm(v);
	gsl_matrix_set_all(m,0.0);
	return v;
}

val egsl_ones(size_t rows, size_t columns) {
	val v = egsl_alloc(rows,columns);
	gsl_matrix * m = egsl_gslm(v);
	gsl_matrix_set_all(m,1.0);
	return v;
}

val egsl_vers(double theta){
	double v[2] = { cos(theta), sin(theta)};
	return egsl_vFa(2,v);
}
