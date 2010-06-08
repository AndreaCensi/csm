/*
Copyright (c) 2004, Tim Bailey
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the Player Project nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/* Simple 2D geometric operations with points, poses, and lines.
 *
 * These algorithms were worked out by me from first principles several years
 * ago. They work ok, but are not particularly good implementations. I recommend
 * the geometric source-code and resources by David Eberly on the "Magic Software"
 * website: www.magic-software.com.
 *
 * Tim Bailey 2004.
 */
#ifndef GEOMETRY_2D_H_
#define GEOMETRY_2D_H_

#include <vector>

namespace Geom2D {

//
// Basic Structures
//

struct Point 
{
	double x;
	double y;
	short int laser_index;
};

struct Pose 
{
	Point p;
	double phi;
};

struct Line {
	Point first;
	Point second;
};

//
// Utility functions
//

const double PI = 3.14159265358979;

inline 
double sqr(double x) { return x*x; }

inline 
double abs(double x) { return (x<0.) ? -x : x; }

inline
double round(double x) { 
	return (x<0.) ? -static_cast<int>(0.5-x) : static_cast<int>(0.5+x); 
}
/*
template<class T>
inline
void swap(T& a, T& b) 
{
	T tmp(a);
	a = b;
	b = tmp;
}
*/
inline
double pi_to_pi(double angle) { // normalise an angle to within +/- PI
	while (angle < -PI)
		angle += 2.*PI;
	while (angle > PI)
		angle -= 2.*PI;
	return angle;
}

//
// Point and Pose algorithms
//

inline 
double dist_sqr(const Point& p, const Point& q) { // squared distance between two Points
	return (sqr(p.x-q.x) + sqr(p.y-q.y));
}

double dist(const Point& p, const Point& q);
Pose compute_relative_pose(const std::vector<Point>& a, const std::vector<Point>& b);

//
// Line algorithms
//

bool intersection_line_line (Point& p, const Line& l, const Line& m);
double distance_line_point (const Line& lne, const Point& p);
void intersection_line_point(Point& p, const Line& l, const Point& q);

//
// Basic transformations on 2-D Points (x,y) and Poses (x,y,phi).
//

class Transform2D {
public:
	Transform2D(const Pose& ref);

	void transform_to_relative(Point &p);
	void transform_to_relative(Pose &p);
	void transform_to_global(Point &p);
	void transform_to_global(Pose &p);

private:
	const Pose base;
	double c;
	double s;
};

inline
void Transform2D::transform_to_relative(Point &p) {
	p.x -= base.p.x; 
	p.y -= base.p.y;
	double t(p.x);
	p.x = p.x*c + p.y*s;
	p.y = p.y*c -   t*s;
}

inline
void Transform2D::transform_to_global(Point &p) {
	double t(p.x); 
	p.x = base.p.x + c*p.x - s*p.y;
	p.y = base.p.y + s*t   + c*p.y;
}

inline
void Transform2D::transform_to_relative(Pose &p) {
	transform_to_relative(p.p);
	p.phi= pi_to_pi(p.phi-base.phi);
}

inline
void Transform2D::transform_to_global(Pose &p) {
	transform_to_global(p.p);
	p.phi= pi_to_pi(p.phi+base.phi);
}

} // namespace Geom2D

#endif
