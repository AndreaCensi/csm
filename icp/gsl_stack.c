#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include "gsl_stack.h"

gsl_matrix * op1 = 0;
gsl_matrix * op2 = 0;

void alloc(gsl_matrix **result, size_t rows, size_t  columns) {
	if(!*result) {
		*result = gsl_matrix_alloc(rows,columns);
		return;
	}
	
	if((*result)->size1 == rows && (*result)->size2 == columns)
		return;
		
	gsl_matrix_free(*result);
	*result = gsl_matrix_alloc(rows, columns);
}

void alloc2(size_t  rows, size_t  columns) {
	alloc(&op2, rows, columns);
}

void swap(){
	gsl_matrix*t = op2;
	op2=op1;
	op1=t;
}

void gsls_set(const gsl_matrix*m) {
	alloc(&op1, m->size1, m->size2);
	gsl_matrix_memcpy(op1, m);
}

void gsls_trans() {
	alloc2(op1->size2, op1->size1);
	gsl_matrix_transpose_memcpy(op2, op1);
	swap();
}

// Right multiply
void gsls_mult(const gsl_matrix*m) {
	alloc2(op1->size1, m->size2);
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,op1,m,0.0,op2);
	swap();
}

// Multiply transpose
void gsls_mult_t(const gsl_matrix*m) {
	alloc2(op1->size1, m->size1);
	gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,op1,m,0.0,op2);
	swap();
}

// Multiply transpose left
void gsls_mult_left(const gsl_matrix*m) {
	alloc2(m->size1, op1->size2);
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,m,op1,0.0,op2);
	swap();	
}

gsl_matrix* gsls_copy() {
	gsl_matrix* m = gsl_matrix_alloc(op1->size1,op1->size2);
	gsl_matrix_memcpy(m,op1);
	return m;
}

void m_inv2(const gsl_matrix*A, gsl_matrix*invA);

void gsls_inv() {
	alloc2(op1->size1, op1->size2);
	m_inv2(op1,op2);
	swap();
}

double gsls_get_at(size_t  i, size_t  j){
	return gmg(op1, i, j);
}

void m_inv2(const gsl_matrix*A, gsl_matrix*invA) {
	unsigned int n = A->size1;
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
}

double m_det2(const gsl_matrix*A) {
	unsigned int n = A->size1;
	gsl_matrix * m = gsl_matrix_alloc(n,n);
	gsl_matrix_memcpy(m,A);
	gsl_permutation * perm = gsl_permutation_alloc (n);
	int sign;
	gsl_linalg_LU_decomp (m, perm, &sign);
	double det = gsl_linalg_LU_det(m, sign);
	gsl_matrix_free(m);
	return det;
}

