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


/* Nearest-neighbours algorithm for 2D point-sets.
 * Compute the single nearest-neighbour of a point, or the set of k-nearest neighbours.
 *
 * This is a very simple algorithm that is reasonably efficient for planar
 * points. However much faster algorithms are possible.
 *
 * Tim Bailey 2004.
 */

#ifndef NN_HEADER
#define NN_HEADER

#include "geometry2D.h"
#include <vector>

namespace Geom2D {

// Simple 2D k-nearest-neighbours search
//
class SweepSearch {
public:
	enum { NOT_FOUND = -1 };

	SweepSearch(const std::vector<Point> &p, double dmax);

	int query(const Point &q) const;
	std::vector<double>& query(const Point &q, std::vector<int> &idx);

private:	
	struct PointIdx { 
		PointIdx() {}
		PointIdx(const Point &p_, const int& i_) : p(p_), i(i_) {}
		Point p;
		int i; 
	};

	const double limit;
	std::vector<PointIdx> dataset;
	std::vector<double> nndists;

	bool is_nearer(double &d2min, int &idxmin, const Point &q, const PointIdx &pi) const;

	bool insert_neighbour(const Point &q, const PointIdx &pi, 
		std::vector<double> &nndists, std::vector<int> &idx);

	static bool yorder (const PointIdx& p, const PointIdx& q) {
		return p.p.y < q.p.y;
	}
};

}

#endif
