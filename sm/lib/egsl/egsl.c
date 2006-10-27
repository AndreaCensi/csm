#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

#include <assert.h>
#include <math.h>

#include "egsl.h"
#define MAX_VALS 1024
#define MAX_CONTEXTS 1024

#define INVALID (val_from_context_and_index(1000,1000))

struct egsl_variable {
	gsl_matrix * gsl_m;
};

struct egsl_context {
	int nallocated;
	int nvars;
	struct egsl_variable vars[MAX_VALS]; 
};

int cid=0;
struct egsl_context egsl_contexts[MAX_CONTEXTS];


int egsl_first_time = 1;

int egsl_total_allocations = 0;
int egsl_cache_hits = 0;

void error() {
	// TODO: better handling of errors
	
	egsl_print_stats();
	
	assert(0);
}

inline val assemble_val(int cid, int index, gsl_matrix*m) {
	val v; 
		v.cid=cid; 
		v.index=index; 
		v.gslm = m; 
	return v;
}
inline int its_context(val v) {
	return v.cid;
}

inline int its_var_index(val v) {
	return v.index;
}


#if 0
inline void check_valid_val(val v) { int i = v.cid; v.cid=i;}	
#else
void check_valid_val(val v) {
	int context = its_context(v);
	if(context>cid) {
		printf("Val is from invalid context (%d>%d)\n",context,cid);
		error();
	}
	int var_index = its_var_index(v);
	if(var_index >= egsl_contexts[context].nvars) {
		printf("Val is invalid  (%d>%d)\n",var_index, 
			egsl_contexts[context].nvars);		
		error();
	}
}
#endif

inline gsl_matrix * egsl_gslm(val v) {
	check_valid_val(v);
	return v.gslm;
}


void egsl_push() {
	if(egsl_first_time) {
		int c;
		for(c=0;c<MAX_CONTEXTS;c++) {
			egsl_contexts[c].nallocated = 0;
			egsl_contexts[c].nvars = 0;
		}
		egsl_first_time  = 0;
	}
	cid++;
	assert(cid<MAX_CONTEXTS);// "Maximum number of contexts reached");
}

void egsl_pop() {
	assert(cid>=0);//, "No egsl_push before?");
	egsl_contexts[cid].nvars = 0;
	cid--;
}

void egsl_print_stats() {
	printf("egsl: total allocations: %d   cache hits: %d\n",	
		egsl_total_allocations, egsl_cache_hits);
//	printf("egsl: sizeof(val) = %d\n",(int)sizeof(val));
	int c; for(c=0;c<MAX_CONTEXTS;c++) {
	//	printf("egsl: context #%d\n ",c);
	//	if(0==egsl_contexts[c].nallocated) break;
		printf("egsl: context #%d allocations: %d active: %d\n",
			c,	egsl_contexts[c].nallocated, 	egsl_contexts[c].nvars);
	}
}

val egsl_alloc(size_t rows, size_t columns) {
	struct egsl_context*c = egsl_contexts+cid;
	
//	if(cid<3)
//	printf("Alloc cid=%d nvars=%d nalloc=%d\n",cid,c->nvars,c->nallocated);
	
	if(c->nvars>=MAX_VALS) {
		fprintf(stderr,"Limit reached, in context %d, nvars is %d\n",cid,c->nvars);
		error();
	}
	int index = c->nvars;
	if(index<c->nallocated) {
		gsl_matrix*m = c->vars[index].gsl_m;
		if(m->size1 == rows && m->size2 == columns) {
			egsl_cache_hits++;
			c->nvars++;
			return assemble_val(cid,index,c->vars[index].gsl_m);
		} else {
			gsl_matrix_free(m);
			egsl_total_allocations++;			
			c->vars[index].gsl_m = gsl_matrix_alloc((size_t)rows,(size_t)columns);
			c->nvars++;
			return assemble_val(cid,index,c->vars[index].gsl_m);
		}
	} else {
		egsl_total_allocations++;
		c->vars[index].gsl_m = gsl_matrix_alloc((size_t)rows,(size_t)columns);
		c->nvars++;
		c->nallocated++;
		return assemble_val(cid,index,c->vars[index].gsl_m);
	}
}

val egsl_alloc_in_context(int context, size_t rows, size_t columns) {
	egsl_total_allocations++;
	struct egsl_context *c = egsl_contexts+context;
	int index = c->nvars;
	c->vars[index].gsl_m = gsl_matrix_alloc((size_t)rows,(size_t)columns);
	c->nvars++;
	return assemble_val(context,index,c->vars[index].gsl_m);
}

/// Creates a copy of v in the previous context.
val egsl_promote(val v) {
	if(cid==0) {
		error();
	}

	gsl_matrix * m = egsl_gslm(v);
	val v2 = egsl_alloc_in_context(cid-1, m->size1, m->size2);
	gsl_matrix * m2 = egsl_gslm(v2);
	gsl_matrix_memcpy(m2, m);
	return v2;
}




void egsl_expect_size(val v, size_t rows, size_t cols) {
	gsl_matrix * m = egsl_gslm(v);
	
	int bad = (rows && (m->size1!=rows)) || (cols && (m->size2!=cols));
	if(bad) {
		fprintf(stderr, "Matrix size is %d,%d while I expect %d,%d",
			(int)m->size1,(int)m->size2,(int)rows,(int)cols);
		
		error();
	}
}


void egsl_print(const char*str, val v) {
	gsl_matrix * m = egsl_gslm(v);
	size_t i,j;
	int context = its_context(v);
	int var_index = its_var_index(v);
	printf("%s =  (%d x %d)  context=%d index=%d\n",
		str,(int)m->size1,(int)m->size2,  context, var_index);

	for(i=0;i<m->size1;i++) {
		if(i==0)
			printf(" [ ");
		else
			printf("   ");
		
		for(j=0;j<m->size2;j++) 
			printf("%f ", gsl_matrix_get(m,i,j));
		
		
		if(i==m->size1-1)
		printf("] \n");
		else
		printf("; \n");
	}	
}

double* egsl_atmp(val v, size_t i, size_t j) {
	gsl_matrix * m = egsl_gslm(v);
	return gsl_matrix_ptr(m,(size_t)i,(size_t)j);
}


double egsl_norm(val v1){
	egsl_expect_size(v1, 0, 1);
	double n=0;
	size_t i;
	gsl_matrix * m = egsl_gslm(v1);
	for(i=0;i<m->size1;i++) {
		double v = gsl_matrix_get(m,i,0);
		n += v * v;
	}
	return sqrt(n);
}

double egsl_atv(val v1,  size_t i){
	return *egsl_atmp(v1, i, 0);
}

double egsl_atm(val v1, size_t i, size_t j){
	return *egsl_atmp(v1, i, j);
}



