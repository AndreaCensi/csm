// GPC: A library for the solution of General Point Correspondence problems.
// Copyright (C) 2006 Andrea Censi (andrea at censi dot org)

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_poly.h>

#include "gpc.h"

#define M(matrix, rows, col) gsl_matrix*matrix = gsl_matrix_alloc(rows,col)
#define MF(matrix) gsl_matrix_free(matrix)
#define gmg gsl_matrix_get
#define gms gsl_matrix_set

void m_trans(const gsl_matrix*A, gsl_matrix*A_t);
void m_mult(const gsl_matrix*A, const gsl_matrix*B, gsl_matrix*AB);
void m_add(const gsl_matrix*A, const gsl_matrix*B, gsl_matrix*ApB);
void m_add_to(const gsl_matrix*A, gsl_matrix*B);
void m_scale(double m, gsl_matrix*A);
double m_dot(const gsl_matrix*A,const gsl_matrix*B);
// GSL is a pain to work with. The library DOES NOT HAVE a determinant() function
// or an inv() function: you have to write your own routines.
void m_inv(const gsl_matrix*A, gsl_matrix*invA);
double m_det(const gsl_matrix*A);

double poly_greatest_real_root(unsigned int n, double*);
void m_display(const char*str, gsl_matrix*m);

int gpc_solve(int K, const struct gpc_corr*c, double *x_out) {
	M(bigM,    4,4); M(g,  4,1); M(bigM_k,2,4);
	M(bigM_k_t,4,2); M(C_k,2,2); M(q_k,   2,1);
	M(temp42,  4,2); M(temp44,4,4);	M(temp21, 2,1);
	M(temp41,  4,1); M(temp22,2,2);	M(temp22b,2,2);
	M(temp22c, 2,2); M(temp12,1,2);
		
	gsl_matrix_set_zero(bigM);
	gsl_matrix_set_zero(g);
	int k;
	for(k=0;k<K;k++) {
		gms(bigM_k,0,0,1.0); gms(bigM_k,0,1,0.0); gms(bigM_k,0,2, c[k].p[0]);
		gms(bigM_k,0,3,-c[k].p[1]);
		gms(bigM_k,1,0,0.0); gms(bigM_k,1,1,1.0); gms(bigM_k,1,2,c[k].p[1]);
		gms(bigM_k,1,3,c[k].p[0]);
		gms(C_k,0,0,c[k].C[0][0]); gms(C_k,0,1,c[k].C[0][1]);
		gms(C_k,1,1,c[k].C[1][1]); gms(C_k,1,0,c[k].C[1][0]);
		gms(q_k,0,0,c[k].q[0]);gms(q_k,1,0,c[k].q[1]);
		
		m_trans(bigM_k, bigM_k_t);
		m_mult(bigM_k_t, C_k, temp42);
		m_mult(temp42, bigM_k, temp44);
		m_scale(2.0, temp44);
		m_add_to(temp44, bigM);
		
		m_mult(C_k, q_k, temp21);
		m_mult(bigM_k_t, temp21, temp41);
		m_scale(-2.0, temp41);
		m_add_to(temp41, g);	

		if(0) {
			m_display("bigM_k",bigM_k);
			m_display("q_k",q_k);
			m_display("C_k",C_k);
			m_display("now M is ",bigM);
			m_display("now g is ",g);
		}
	}
	
	if(0) {
		m_display("bigM",bigM);
		m_display("g",g);
	}
	
	M(mA,2,2); gms(mA,0,0, gmg(bigM,0,0)); gms(mA,0,1, gmg(bigM,0,1));
	           gms(mA,1,0, gmg(bigM,1,0)); gms(mA,1,1, gmg(bigM,1,1));
	M(mB,2,2); gms(mB,0,0, gmg(bigM,0,2)); gms(mB,0,1, gmg(bigM,0,3));
	           gms(mB,1,0, gmg(bigM,1,2)); gms(mB,1,1, gmg(bigM,1,3));
	M(mD,2,2); gms(mD,0,0, gmg(bigM,2,2)); gms(mD,0,1, gmg(bigM,2,3));
	           gms(mD,1,0, gmg(bigM,3,2)); gms(mD,1,1, gmg(bigM,3,3));

	M(mS,2,2); M(mSa,2,2);

	 
	//	mS = mD - mB.trans * mA.inv * mB;
	// temp22b = inv(A)
	m_inv(mA, temp22b); 
	// temp22c = inv(A) * mB
	m_mult(temp22b, mB, temp22c);
	// temp22 = mB'
	m_trans(mB, temp22); 
	m_mult(temp22, temp22c, temp22b); 
	m_scale(-1.0,temp22b);
	m_add(mD,temp22b,mS);
	
	// mSa = mS.inv * mS.det;
	m_inv(mS, mSa);
	m_scale(m_det(mS), mSa);
	
	if(0) {
		m_display("mA",mA);
		m_display("mB",mB);
		m_display("mD",mD);
		m_display("mS",mS);
		m_display("mSa",mSa);
	}

	M(g1,2,1);	M(g2,2,1);
	M(g1t,1,2);	M(g2t,1,2);
	M(mAi,2,2);	M(mBt,2,2);
	
	gms(g1,0,0,gmg(g,0,0));
	gms(g1,1,0,gmg(g,1,0));
	gms(g2,0,0,gmg(g,2,0));
	gms(g2,1,0,gmg(g,3,0));
	m_trans(g1, g1t);
	m_trans(g2, g2t);
	m_trans(mB, mBt);
	m_inv(mA, mAi);

	M(m1t,1,2);	M(m1,2,1);
	M(m2t,1,2);	M(m2,2,1);
	M(m3t,1,2);	M(m3,2,1);

	// m1t = g1t*mAi*mB
	m_mult(g1t,mAi,temp12);
	m_mult(temp12,mB,m1t);

	m_trans(m1t,m1);
	// m2t = m1t*mSa
	m_mult(m1t,mSa,m2t);
	m_trans(m2t,m2);
	// m3t = g2t*mSa
	m_mult(g2t,mSa,m3t);
	m_trans(m3t,m3);
	
	double p[3] = {
		  m_dot(m2t,m2) - 2*m_dot(m2t,m3) +   m_dot(m3t,m3),
		4*m_dot(m2t,m1) - 8*m_dot(m2t,g2) + 4*m_dot(g2t,m3),
		4*m_dot(m1t,m1) - 8*m_dot(m1t,g2) + 4*m_dot(g2t,g2)};

	double l[3] = {m_det(mS), 2*gmg(mS,0,0)+2*gmg(mS,1,1), 4};
	
	// q = p - l^2
	double q[5] = {p[0]-(l[0]*l[0]), p[1]-(2*l[1]*l[0]), 
		p[2]-(l[1]*l[1]+2*l[0]*l[2]), -(2*l[2]*l[1]), -(l[2]*l[2])};
	
	double lambda = poly_greatest_real_root(5,q);
	
	if(0) {
		printf("p = %f %f %f \n", p[2], p[1], p[0]);
		printf("l = %f %f %f \n", l[2], l[1], l[0]);
		printf("q = %f %f %f %f %f \n", q[4],  q[3],  q[2], q[1], q[0]);
		printf("lambda = %f \n", lambda);
	}	

	M(W,4,4); gsl_matrix_set_zero(W); gms(W,2,2,1.0); gms(W,3,3,1.0);
	M(x,4,1);
	
	m_scale(2*lambda, W);
	gsl_matrix_add(bigM,W);
	m_inv(bigM, temp44);
	m_mult(temp44, g, x);
	m_scale(-1.0, x);
	
	x_out[0] = gmg(x,0,0);
	x_out[1] = gmg(x,1,0);
	x_out[2] = atan2(gmg(x,3,0),gmg(x,2,0));
	
	if(0) {
		printf("x =  %f  %f %f deg\n", x_out[0], x_out[1],x_out[2]*180/M_PI);
	}
	
	MF(mA); MF(mB); MF(mD); MF(mS); MF(mSa);
	MF(m1t);	MF(m1);	MF(m2t);	MF(m2);
	MF(m3t);	MF(m3);	MF(W); 	MF(x);
	MF(bigM); MF(g); MF(bigM_k);
	MF(bigM_k_t); MF(C_k); MF(q_k);
	MF(temp42); MF(temp44);	MF(temp21);
	MF(temp41); MF(temp22);	MF(temp22b);
	MF(temp22c); MF(temp12);
	MF(g1);	MF(g2);
	MF(g1t);	MF(g2t);
	MF(mAi);	MF(mBt);
	return 0;
}


void m_trans(const gsl_matrix*A, gsl_matrix*A_t){
	gsl_matrix_transpose_memcpy(A_t,A);
}

void m_mult(const gsl_matrix*A, const gsl_matrix*B, gsl_matrix*AB){
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,A,B,0.0,AB);
}

void m_add_to(const gsl_matrix*A, gsl_matrix*B){
	gsl_matrix_add(B, A);
}

void m_scale(double m, gsl_matrix*A){
	gsl_matrix_scale(A,m);
}

void m_add (const gsl_matrix*A, const gsl_matrix*B, gsl_matrix*ApB){
	gsl_matrix_memcpy(ApB,A);
	gsl_matrix_add(ApB,B);	
}

void m_inv(const gsl_matrix*A, gsl_matrix*invA) {
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

double m_det(const gsl_matrix*A) {
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

double m_dot(const gsl_matrix*A,const gsl_matrix*B) {
	double sum = 0;
	unsigned int j;
	for(j=0;j<A->size2;j++)
		sum += gmg(A,0,j)*gmg(B,j,0);
	return sum;
}

double poly_greatest_real_root(unsigned int n, double*a) {
	double z[n*2];
	gsl_poly_complex_workspace * w  = gsl_poly_complex_workspace_alloc(n);
	gsl_poly_complex_solve (a, n, w, z);
	gsl_poly_complex_workspace_free (w);
	double lambda = 0;
	unsigned int i;
	for (i = 0; i < n; i++) {
//		printf ("z%d = %+.18f %+.18f\n", i, z[2*i], z[2*i+1]);
		// XXX ==0 is bad
		if( (z[2*i+1]==0) && (z[2*i]>lambda))
			lambda = z[2*i];
	}
	return lambda;
}

void m_display(const char*str, gsl_matrix*m) {
	printf("%s= \n", str);
	unsigned int i,j;
	for(i=0;i<m->size1;i++) {
		printf("   ");
		for(j=0;j<m->size2;j++)
			printf("%f ", gmg(m,i,j));
		printf("\n");
	}
}

