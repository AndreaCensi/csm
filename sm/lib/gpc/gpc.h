/*
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
*/

#ifndef H_GENERAL_POINT_CORRESPONDENCE
#define H_GENERAL_POINT_CORRESPONDENCE

struct gpc_corr {
	double p[2];
	double q[2];

	double C[2][2];
	
	int valid;
};

/**
// This program solves the general point correspondences problem:
// to find a translation $t$ and rotation $\theta$ that minimize
//
//  \sum_k (rot(theta)*c[k].p+t-c[k].q)' * c[k].C * (rot(theta)*c[k].p+t-c[k].q)
//
// (see the attached documentation for details).
*/


#define TRACE_ALGO 0

/* Set to 1 to force a check that C is positive semidef.
   Note that you will have some numerical errors! */
#define GPC_CHECK_SEMIDEF 0

/** if c[k].valid is 0, the correspondence is not used */
int gpc_solve(int K, const struct gpc_corr*, 
	const double*x0, const double *cov_x0,
	double *x);

/* Some utilities functions */

	/** Computes error for a single correspondence */
	double gpc_error(const struct gpc_corr*co, const double*x);

	double gpc_total_error(const struct gpc_corr*co, int n, const double*x);
#endif

