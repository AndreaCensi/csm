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

/* Iterated closest point (ICP) algorithm.
 * 
 * This is a very simple implementation of ICP, with one simple extension where the
 * association may be chosen as an interpolation between two nearest neighbours rather
 * than just point-to-point with the nearest neighbour.
 *
 * A lot of extensions to this basic algorithm are possible. For example,
 *	1. Record at each iteration the set of NN associations for each observation. 
 *	If these do not change, then ICP has converged completely.
 *	
 *	2. Various speed ups given in the following papers:
 *
 *	(a) P.J. Besl and N.D. McKay. A method for registration of 3-D shapes. IEEE 
 *	Transactions on Pattern Analysis and Machine Intelligence, 14(2):239256, 1992.
 *
 *	(b) F. Lu and E. Milios. Robot pose estimation in unknown environments by matching
 *	2D range scans. Journal of Intelligent and Robotic Systems, 18:249275, 1997.
 *
 *	(c) S. Rusinkiewicz and M. Levoy. Efficient variants of the ICP algorithm. In Third
 *	International Conference on 3D Digital Imaging and Modeling, pages 145152, 2001.
 *
 *	3. Methods for estimating the error introduced by point-wise correspondence in 
 *	the paper (b) above and also:
 *
 *	S.T. P?ster, K.L. Kriechbaum, S.I. Roumeliotis, and J.W. Burdick. Weighted range
 *	sensor matching algorithms for mobile robot displacement estimation. In IEEE 
 *	International Conference on Robotics and Automation, 2002.
 *
 * Tim Bailey 2004.
 */
#include <vector>
#include "geometry2D.h"
#include "nn.h"
#include <memory>
using namespace std;
namespace Geom2D {

class ICP {
public:
	ICP();
	~ICP();
	Pose align(std::vector<Point> , std::vector<Point>,Pose , double , int , bool );
	const std::vector<Point> get_ref_points() { return b; }
	const std::vector<Point> get_obs_points() { return a; }
	bool warning_misalign;
private:
	vector<Point>   ref;
	SweepSearch * nn;
	vector<Point> a;
	vector<Point> b;
	vector<int> index;
};

} // namespace Geom2D 
