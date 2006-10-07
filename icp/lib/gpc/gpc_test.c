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

#include <stdio.h>
#include <math.h>
#include "gpc.h"

#define deg2rad(x) (x*M_PI/180)

int main() {
	// This is the true roto-translation
	double theta = deg2rad(4);
	double t[2] = {0.3,-0.2};
	printf("x_true =  %f  %f  %f deg\n", t[0], t[1], theta*180/M_PI);	
	
	double p[5][2] = {{1,0},{0,1},{-1,0},{2,1},{4,2}};
	double alpha[5] = {deg2rad(0), 
		deg2rad(10), deg2rad(20), deg2rad(50),deg2rad(-20)};

	struct gpc_corr c[5];
	int k;
	for(k=0;k<5;k++) {
		c[k].p[0] = p[k][0];
		c[k].p[1] = p[k][1];
		c[k].q[0] = t[0] + p[k][0]*cos(theta) - p[k][1]*sin(theta);
		c[k].q[1] = t[1] + p[k][0]*sin(theta) + p[k][1]*cos(theta);
		c[k].C[0][0] = cos(alpha[k])*cos(alpha[k]);
		c[k].C[0][1] = c[k].C[1][0] = cos(alpha[k])*sin(alpha[k]);
		c[k].C[1][1] = sin(alpha[k])*sin(alpha[k]);
	}

	double x[3];
	gpc_solve(5,c,x);

	printf("estimated x =  %f  %f  %f deg\n", x[0], x[1],x[2]*180/M_PI);
	return 0;
}

