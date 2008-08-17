#include "raytracer.h"

namespace RayTracer {
	using namespace std;


	bool Segment::ray_tracing(const double p[2], const double direction, 
	 	double& range, double &alpha) const {

		int found = segment_ray_tracing(this->p0, this->p1, p, direction, &range);

		if(found) {
			alpha = segment_alpha(this->p0, this->p1);

			if( cos(alpha-direction) < 1 )
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
				if(range<champion_range || champion==-1) {
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
	
	/*
	
	LaserData Map::rayTracing(
		Point2od pose, double fov_rad, int nrays, double maxReading) 
	{
		LaserData ld;
		ld.odometry = pose;
		ld.estimate = pose;
		ld.laserPose = Point2od(0,0,0);
		ld.minReading = 0;
		ld.maxReading = maxReading;
		ld.minTheta = -fov_rad/2;
		ld.maxTheta = +fov_rad/2;
		
		//cout << "Ray tracing: maxreading = " << maxReading << endl;
		
		for(int i=0;i<nrays;i++) {
			LaserData::LaserPoint lp;
			lp.theta = ld.minTheta + ((ld.maxTheta-ld.minTheta)*i)/nrays;
			
			bool valid = laserIncidence(
				lp.reading, lp.trueAlpha,
				Point2od(pose.x,pose.y,pose.theta+lp.theta), maxReading);
			
			if(!valid) 
				lp.markInvalid();
			
			//cout << "ray " << i << " valid = " << lp.isValid() <<
			//	" alpha = " << lp.trueAlpha << " reading = " << lp.reading << endl;
			ld.points.push_back(lp);
		}
		return ld;
	}
	
	void Map::ray_tracing(
		Point2od pose, double fov_rad, int nrays, double maxReading,
		double*reading, double*theta, double *alpha, int*valid) 
	{	
		double minTheta = -fov_rad/2;
		double maxTheta = +fov_rad/2;
		
		for(int i=0;i<nrays;i++) {
			theta[i] = minTheta + ((maxTheta-minTheta)*i)/nrays;
			
			bool is_valid = laserIncidence(
				reading[i], alpha[i],
				Point2od(pose.x, pose.y, pose.theta + theta[i]), 
				maxReading);
			
			if(!is_valid) {
				valid[i] = 0;
				reading[i] = alpha[i] = NAN;
			} else {
				valid[i] = 1;
			}
			
		}
	}
	
	*/
} // namespace SimpleMap
