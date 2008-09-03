#ifndef MAPAC_H
#define MAPAC_H

#include <iostream>
#include <vector>
#include <string>

namespace RayTracer {
	
	using namespace std;
	
	struct Material {
		string name;
		
		/* 0: transparent; 1: solid; in between: randomly */
		double infrared_solid;
	};
	
	struct Stuff {
		int group;
		
		Material * material;
		
		/** Ray tracing with incidence. */
		virtual bool ray_tracing(const double p[2], const double direction, double& out_distance, double &out_alpha) const = 0; 	
		virtual ~Stuff(){};
	};
	
	struct Segment: public Stuff {
		double p0[2], p1[2];
		
		Segment() {}
		virtual ~Segment(){};
		
		Segment(double x0,double y0,double x1,double y1) {
			p0[0] = x0; p0[1] = y0;
			p1[0] = x1; p1[1] = y1;
		}
		
		
		bool ray_tracing(const double p[2], const double direction, double& out_distance, double &out_alpha) const;
	};

	struct Circle: public Stuff {
		virtual ~Circle(){};
		
		double p[2], radius;
		
		bool ray_tracing(const double p[2], const double direction,  double& out_distance, double &out_alpha) const;
	};
	
	struct Environment  {
		std::vector<Stuff*> stuff;

		bool ray_tracing(const double p[2], const double direction,  double& out_distance, double &out_alpha, int*stuff_id) const ;
				
	};
	

	
}

#endif
