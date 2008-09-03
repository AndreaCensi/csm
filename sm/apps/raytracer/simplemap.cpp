#include "raytracer.h"

namespace RayTracer {
	using namespace std;


	bool Segment::ray_tracing(const double p[2], const double direction, 
	 	double& range, double &alpha) const {

		int found = segment_ray_tracing(this->p0, this->p1, p, direction, &range);

		if(found) {
			alpha = segment_alpha(this->p0, this->p1);
	
			/* alpha and direction should have versors with negative dot product */
			if( cos(alpha)*cos(direction) + sin(alpha)*sin(direction) > 0 )
				alpha = alpha + M_PI;

			alpha = normalize_0_2PI(alpha);

			return true;
		} else {
			alpha = NAN;
			return false;
		}		
	};

	bool Environment::ray_tracing(const double p[2], const double direction,  double& out_distance, double &out_alpha, int*stuff_id) const {
	
		int champion = -1;
		double champion_range, champion_alpha;
		for(size_t i=0;i<stuff.size();i++) {
			Stuff * s = stuff.at(i);
			
			double range, alpha;
			if(s->ray_tracing(p,direction,range,alpha)){
				if(champion==-1 || range<champion_range) {
					champion = i;
					champion_range = range;
					champion_alpha = alpha;
				}
			}
			
		}
		
		if(champion != -1) {
			*stuff_id = champion;
			out_distance = champion_range;
			out_alpha = champion_alpha;
		
			return true;
		} else {
			return false;
		}
	}
} // namespace SimpleMap
