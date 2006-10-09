#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <assert.h>
#include <math.h>

#include "easy_gsl.h"
#define MAX_VALS 1024
#define MAX_CONTEXTS 1024

#define INVALID (val_from_context_and_index(1000,1000))

struct egsl_variable {
	gsl_matrix * gsl_m;
};

struct egsl_context {
	int nvars;
	struct egsl_variable vars[MAX_VALS]; 
};

int cid=0;
struct egsl_context egsl_contexts[MAX_CONTEXTS];

void error() {
	
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
}

void egsl_pop() {
	assert(cid>=0);//, "No egsl_push before?");
	egsl_free_context(cid);
	cid--;
}

int its_context(val v) {
	return (v>>16) & 0xffff;
}

int its_var_index(val v) {
	return v & 0xffff;
}

val val_from_context_and_index(int cid, int index) {
	return (cid<<16) | index;
}

void check_valid_val(val v) {
	int context = its_context(v);
	if(context>cid) {
		printf("Val is from invalid context (%d>%d)\n",context,cid);
	}
	int var_index = its_var_index(v);
	if(var_index >= egsl_contexts[context].nvars) {
		printf("Val is invalid  (%d>%d)\n",var_index, egsl_contexts[context].nvars);		
	}
	error();
}

struct egsl_variable * get_var(val v) {
	check_valid_val(v);
	int context = its_context(v);
	int var_index = its_var_index(v);
	return &(egsl_contexts[context].vars[var_index]);
}

gsl_matrix * egsl_gslm(val v) {
	return get_var(v)->gsl_m;
}

void egsl_print(const char*str, val v) {
	gsl_matrix * m = egsl_gslm(v);
	size_t i,j;
	printf("%s = \n",str);
	for(i=0;i<m->size1;i++) {
		printf("   ");
		for(j=0;j<m->size2;j++) {
			printf("%f ", gsl_matrix_get(m,i,j));
		}
		printf("\n");
	}	
}

val egsl_alloc(int rows, int columns) {
	struct egsl_context*c = egsl_contexts+cid;
	if(c->nvars>=MAX_VALS) {
		printf("Limit reached\n");
		error();
	}
	int index = c->nvars;
	c->vars[index].gsl_m = gsl_matrix_alloc((size_t)rows,(size_t)columns);
	val v = val_from_context_and_index(cid,index);
	c->nvars++;
	return v;
}

double* egsl_atmp(val v, int i, int j) {
	gsl_matrix * m = egsl_gslm(v);
	return gsl_matrix_ptr(m,(size_t)i,(size_t)j);
}

val egsl_vFda(int rows, int cols, const double *a) {
	val v = egsl_alloc(rows, cols);

	int i; int j;
	for(i=0;i<rows;i++)
	for(j=0;j<cols;j++) {
		*egsl_atmp(v,i,j) = a[i+j*cols];
	}
	return v;
}

val egsl_vFa(int rows, const double*a) {
	val v = egsl_alloc(rows,1);
	int i;
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

val egsl_zeros(int rows, int columns) {
	return INVALID;
}

val egsl_scale(double s, val v){
	return INVALID;
}

val egsl_sum(val v1, val v2){
	return INVALID;
}

val egsl_sum3(val v1, val v2, val v3){
	return INVALID;
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
	return INVALID;
}

val egsl_inverse(val v1){
	return INVALID;
}

double egsl_norm(val v1){
	return INVALID;
}

double egsl_atv(val v1,  int i){
	return INVALID;
}

double egsl_atm(val v1, int i, int j){
	return INVALID;
}


val egsl_sub(val v1,val v2){
	return INVALID;
}


val egsl_compose_col(val v1, val v2){
	return INVALID;
}

val egsl_vers(double theta){
	return INVALID;
}


void egsl_v2da(val v1, double*a){

}

val egsl_vFgslv(const gsl_vector*v){
	return INVALID;
}

val egsl_vFgslm(const gsl_matrix*m){
	return INVALID;
}

gsl_matrix* egsl_v2gslm(val v1){
	return 0;
}

void egsl_add_to(val v1, val v2) {
	
}

void egsl_add_to_col(val v1, int j, val v2) {
	
}

