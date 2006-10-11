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

gsl_matrix * egsl_gslm(val v);

struct egsl_context {
	int nvars;
	struct egsl_variable vars[MAX_VALS]; 
};

int cid=0;
struct egsl_context egsl_contexts[MAX_CONTEXTS];

void error() {
	assert(0);
}

void egsl_expect_size(val v, size_t rows, size_t cols) {
	gsl_matrix * m = egsl_gslm(v);
	
	int bad = (rows && (m->size1!=rows)) || (cols && (m->size2!=cols));
	if(bad) {
		fprintf(stderr, "Matrix size is %d,%d while I expect %d,%d",
			m->size1,m->size2,rows,cols);
		
		error();
	}
}

void egsl_init_context(int i) {
	
}

void egsl_free_context(int i) {
	struct egsl_context * c = egsl_contexts+i;
	int v;
	for(v=0;v<c->nvars;v++) {
		gsl_matrix_free(c->vars[v].gsl_m);
	}
	c->nvars = 0;
}

void egsl_push() {
	cid++;
	assert(cid<MAX_CONTEXTS);// "Maximum number of contexts reached");
	egsl_init_context(cid);
//	printf("Now context is %d\n",cid);
}

void egsl_pop() {
	assert(cid>=0);//, "No egsl_push before?");
	egsl_free_context(cid);
	cid--;
//	printf("pop: Now context is %d\n",cid);
}

int its_context(val v) {
	return (v>>16) & 0x0fff;
}

int its_var_index(val v) {
	return v & 0x0fff;
}

val val_from_context_and_index(int cid, int index) {
	return ((cid& 0x0fff)<<16) | (index& 0x0fff);
}

void check_valid_val(val v) {
	int context = its_context(v);
	if(context>cid) {
		printf("Val %d is from invalid context (%d>%d)\n",v,context,cid);
		error();
	}
	int var_index = its_var_index(v);
	if(var_index >= egsl_contexts[context].nvars) {
		printf("Val %d is invalid  (%d>%d)\n",v,var_index, 
			egsl_contexts[context].nvars);		
		error();
	}
}

gsl_matrix * egsl_gslm(val v) {
	check_valid_val(v);
	int context = its_context(v);
	int var_index = its_var_index(v);
	return egsl_contexts[context].vars[var_index].gsl_m;
}

void egsl_print(const char*str, val v) {
	gsl_matrix * m = egsl_gslm(v);
	size_t i,j;
	int context = its_context(v);
	int var_index = its_var_index(v);
	printf("%s =  (%d x %d) val=%d context=%d index=%d\n",
		str,m->size1,m->size2, v, context, var_index);

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

val egsl_alloc(size_t rows, size_t columns) {
	struct egsl_context*c = egsl_contexts+cid;
	if(c->nvars>=MAX_VALS) {
		printf("Limit reached\n");
		error();
	}
	int index = c->nvars;
	c->vars[index].gsl_m = gsl_matrix_alloc((size_t)rows,(size_t)columns);
	val v = val_from_context_and_index(cid,index);
	c->nvars++;
	
	//printf("Allocated %d\n",v);
	return v;
}

double* egsl_atmp(val v, size_t i, size_t j) {
	gsl_matrix * m = egsl_gslm(v);
	return gsl_matrix_ptr(m,(size_t)i,(size_t)j);
}

val egsl_vFda(size_t rows, size_t cols, const double *a) {
	val v = egsl_alloc(rows, cols);

	size_t i; size_t j;
	for(i=0;i<rows;i++)
	for(j=0;j<cols;j++) {
		*egsl_atmp(v,i,j) = a[i+j*cols];
	}
	return v;
}

val egsl_vFa(size_t rows, const double*a) {
	val v = egsl_alloc(rows,1);
	size_t i;
	for(i=0;i<rows;i++)
		*egsl_atmp(v,i,0) =  a[i];
	return v;
}

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

val egsl_copy_val(val v1) {
	gsl_matrix * m1 = egsl_gslm(v1);
	val v2 = egsl_alloc(m1->size1,m1->size2);
	gsl_matrix * m2 = egsl_gslm(v2);
	gsl_matrix_memcpy(m2,m1);
	return v2;
}

val egsl_scale(double s, val v1){
	val v2 = egsl_copy_val(v1);
	gsl_matrix * m2 = egsl_gslm(v2);
	gsl_matrix_scale(m2, s);
	return v2;
}

val egsl_sum(val v1, val v2){
	gsl_matrix * m1 = egsl_gslm(v1);
	gsl_matrix * m2 = egsl_gslm(v2);
	val v3 = egsl_alloc(m1->size1,m1->size2);
	gsl_matrix * m3 = egsl_gslm(v3);
	gsl_matrix_memcpy(m3,m1);
	gsl_matrix_add(m3,m2);	
	return v3;
}

val egsl_sum3(val v1, val v2, val v3){
	return egsl_sum(v1, egsl_sum(v2,v3));
}

val egsl_mult(val v1, val v2){
	gsl_matrix * a = egsl_gslm(v1);
	gsl_matrix * b = egsl_gslm(v2);
	val v = egsl_alloc(a->size1,b->size2);
	gsl_matrix * ab = egsl_gslm(v); 
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,a,b,0.0,ab);
	return v;
}

val egsl_transpose(val v1){
	gsl_matrix * m1 = egsl_gslm(v1);
	val v2 = egsl_alloc(m1->size2,m1->size1);
	gsl_matrix * m2 = egsl_gslm(v2);
	gsl_matrix_transpose_memcpy(m2,m1);
	return v2;
}

val egsl_inverse(val v1){
	gsl_matrix*A = egsl_gslm(v1);
	val v2 = egsl_alloc(A->size1,A->size1);
	gsl_matrix*invA = egsl_gslm(v2);
	size_t n = A->size1;
	gsl_matrix * m = gsl_matrix_alloc(n,n);
	gsl_matrix_memcpy(m,A);
	gsl_permutation * perm = gsl_permutation_alloc (n);
	// Make LU decomposition of matrix m
	int s;
	gsl_linalg_LU_decomp (m, perm, &s);
	// Invert the matrix m
	gsl_linalg_LU_invert (m, perm, invA);
	gsl_permutation_free(perm);
	gsl_matrix_free(m);
	return v2;
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

val egsl_sub(val v1,val v2){
	return egsl_sum(v1, egsl_scale(-1.0,v2));
}

val egsl_compose_col(val v1, val v2){
	gsl_matrix *m1 = egsl_gslm(v1);
	gsl_matrix *m2 = egsl_gslm(v2);
	egsl_expect_size(v2, 0, m1->size2);
	val v3 = egsl_alloc(m1->size1+m2->size1,m1->size2);
	gsl_matrix *m3 = egsl_gslm(v3);
	size_t i,j;
	for(j=0;j<m1->size2;j++) {
		for(i=0;i<m1->size1;i++)
			gsl_matrix_set(m3, i, j, gsl_matrix_get(m1,i,j));
		
		for(i=0;i<m2->size1;i++)
			gsl_matrix_set(m3, m1->size1+i, j, gsl_matrix_get(m2,i,j));
	}
	return v3;
}

val egsl_compose_row(val v1, val v2){
	gsl_matrix *m1 = egsl_gslm(v1);
	gsl_matrix *m2 = egsl_gslm(v2);
	egsl_expect_size(v2, m1->size1, 0);
	val v3 = egsl_alloc(m1->size1, m1->size2 + m2->size2);
	gsl_matrix *m3 = egsl_gslm(v3);
	size_t i,j;
	for(i=0;i<m1->size1;i++) {
		for(j=0;j<m1->size2;j++) 
			gsl_matrix_set(m3, i, j, gsl_matrix_get(m1,i,j));
		
		for(j=0;j<m2->size2;j++) 
			gsl_matrix_set(m3, i, m1->size2+j, gsl_matrix_get(m2,i,j));
	}
	return v3;
}

val egsl_vers(double theta){
	double v[2] = { cos(theta), sin(theta)};
	return egsl_vFa(2,v);
}


void egsl_v2da(val v, double*a){
	gsl_matrix *m = egsl_gslm(v);
	size_t i,j;
	for(i=0;i<m->size1;i++)
		for(j=0;j<m->size2;j++)
			a[j*m->size1 +i] = gsl_matrix_get(m,i,j);
}

val egsl_vFgslv(const gsl_vector*vec){
	val v = egsl_alloc(vec->size,1);
	size_t i;
	for(i=0;i<vec->size;i++)
		*egsl_atmp(v,i,0) = gsl_vector_get(vec,i);
	return v;
}

val egsl_vFgslm(const gsl_matrix*m){
	val v = egsl_alloc(m->size1,m->size2);
	gsl_matrix * m2 = egsl_gslm(v);
	gsl_matrix_memcpy(m2,m);
	return v;
}

gsl_matrix* egsl_v2gslm(val v){
	gsl_matrix * m = egsl_gslm(v); 
	gsl_matrix * m2 = gsl_matrix_alloc(m->size1,m->size2);
	gsl_matrix_memcpy(m2,m);
	return m;
}

void egsl_add_to(val v1, val v2) {
	gsl_matrix * m1 = egsl_gslm(v1); 
	gsl_matrix * m2 = egsl_gslm(v2);
	gsl_matrix_add(m1,m2);	
}

void egsl_add_to_col(val v1, size_t j, val v2) {
//	egsl_print("m1",v1);
//	egsl_print("m2",v2);
	gsl_matrix * m1 = egsl_gslm(v1); 
	gsl_matrix * m2 = egsl_gslm(v2);
	
//	printf("m1 size = %d,%d j = %d\n",m1->size1,m1->size2,j);
	egsl_expect_size(v2, m1->size1, 1);
	size_t i;
	for(i=0;i<m1->size1;i++) {
		*gsl_matrix_ptr(m1, i, j) += gsl_matrix_get(m2,i,0);
	}
}


