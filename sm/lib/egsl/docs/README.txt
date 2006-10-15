Easy GSL: Making GSL easy for simple tasks.
=======================================

This is a small C library that I wrote for making simple matrix 
computations easy. It is based on 
[GSL, the GNU Scientific Library](http://www.gnu.org/software/gsl/).

Albeit very powerful, the GSL is definitely not user-friendly for
making simple matrix computations.

Instead, EGSL will try to fool you into thinking that you are using Matlab.
Yes, it's that easy! You can forget all of that `gsl_matrix_alloc`
and `gsl_matrix_free`.

This is not designed for large matrixes.

### Download 

Download from [http://purl.org/censi/2006/egsl](http://purl.org/censi/2006/egsl).


The two main features
---------------------

### Feature #1: Automatic (de)allocation of matrixes

Its main feature is that matrixes are automatically allocated and 
deallocated. For example, you can write:

	egsl_push();
	
	val v1 = zeros(10,10);
	val v2 = ones(10,10);
	
	val v3 = sum(v1, v2);
	egsl_print("v1+v2=", v3);
	
	egsl_pop();

and not worry about (de)allocalization.


### Feature #2: Caching of matrixes

This feature makes EGSL faster than any other C++ equivalent that
uses objects. Consider this code:

	egsl_push();
	
	val v1 = zeros(10,10);
	
	for(int i=0;i<1000000;i++) {
		
		egsl_push();
		
			val v2 = zeros(10,10);
			// make some operation on v2
			....
			// add v2 to v1
			add_to(v1, v2);
		
		egsl_pop();
		
	}
	
	egsl_pop();
	
	
	// Prints statistics about EGSL's usage of memory
	egsl_print_stats();

The output of this program is:

	egsl: total allocations: 2   cache hits:999999

Even though the loop executes one million times, the total number
of matrix allocations is 2. Note that there is an inner context.
When the loop runs the first time, the `gsl_matrix` for `v2` is allocated.
However, when the `egsl_pop()` is reached, this matrix is not deallocated.
When the loop runs the second time, EGSL detects that you already
allocated a matrix of the requested size and re-utilizes the memory.

Usage
-----

For EGSL to work, you must remember some simple rules:

1. Always include your code between a pair of `egsl_push`/`egsl_pop` calls.

2. All values are returned as `val` structs: `val` object are not valid outside of the context they are created into (unless you tell EGSL).

Useful macros
-------------



Available operations
--------------------








