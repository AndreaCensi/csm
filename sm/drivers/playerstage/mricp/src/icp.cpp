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
 * Tim Bailey 2004.
 */
#include "icp.h"
#include <cassert>
#include <iostream>
using namespace std;

namespace Geom2D
{
	ICP::ICP()
		: warning_misalign(true)
		{
			index.push_back(0);
			index.push_back(0);
		};
	ICP::~ICP()
		{
			a.clear();
			b.clear();
			index.clear();
			ref.clear();
		}

Pose ICP::align(vector<Point> reference, vector<Point> obs,Pose init, double gate, int nits, bool interp)
{
	ref = reference;
	nn = new SweepSearch(reference, gate);
	Pose pse = init;
	double gate_sqr = sqr(gate);
	int size_obs = obs.size();

	 // used if interp == true
	while (nits-- > 0)
	{
		Transform2D tr(pse);
		a.clear();
		b.clear();
		// For each point in obs, find its NN in ref
		for (int i = 0; i < size_obs; ++i)
		{
			Point p = obs[i];
			tr.transform_to_global(p); // transform obs[i] to estimated ref coord-frame

			Point q;
			// simple ICP
			if (interp == false)
			{
				int idx = nn->query(p);
				if (idx == SweepSearch::NOT_FOUND)
					continue;

				q = ref[idx];
			}
			// ICP with interpolation between 2 nearest points in ref
			else
			{
				(void) nn->query(p, index);
				assert(index.size() == 2);
				if (index[1] == SweepSearch::NOT_FOUND)
					continue;

				Line lne;
				lne.first  = ref[index[0]];
				lne.second = ref[index[1]];
				intersection_line_point(q, lne, p);
			}
			if (dist_sqr(p,q) < gate_sqr) // check if NN is close enough
			{
				a.push_back(obs[i]);
				b.push_back(q);
			}
		}
		//cout<<"\nObs size:"<<obs.size()<<" Ref:"<<ref.size()<<" Paired set size:"<<a.size();
		//If Less than half of the Observation is paired => not good alignment
		if( a.size() < obs.size()/2.0)
		{
			if (this->warning_misalign)
				cout<<"\n Detected Possible misalignment --- Skipping this Laser Set! Gate is:"<<gate;
			pse.p.x = -1 ; pse.p.y=-1 ; pse.phi = -1;
			break;
		}
		if (a.size() > 2)
			pse = compute_relative_pose(a, b); // new iteration result
	}
	delete nn;
	return pse;
}

} // namespace Geom2D
