

/** This is very close (but *less* than) to sin(x), for 
    x in (0, PI/2). It's a 5° degree taylor expansion. */
INLINE double mysin(double x) {
	if(x > 2) return sin(x);
	const double a = -1.0/6.0;
	const double b = +1.0/120.0;
	double x2 = x*x;
	return x * (.99 + x2 * ( a + b * x2));
}
