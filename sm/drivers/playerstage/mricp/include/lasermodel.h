/***************************************************************************
 *   Copyright (C) 2005 by Tarek Taha                                      *
 *   tataha@eng.uts.edu.au                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef LASERMODEL_H_
#define LASERMODEL_H_
// Info for a single range measurement
typedef struct
{
	double range, bearing;
}	laser_range_t;

class LaserModel
{
	private:
	  	mapgrid * * map; 	// Pointer to the OG map
	  	double range_cov;	// Covariance in the range reading
	  	double range_bad;	// Probability of spurious range readings
	  	// Pre-computed laser sensor model
	  	int lut_size;
	  	double  lut_res;
	  	double *lut_probs;
	  	int range_count;
	  	laser_range_t *ranges;
	public :
		void 	ClearRanges();
		void 	AddRange(double,double);
		void 	PreCompute();
		double 	RangeProb(double,double);
		double 	PoseProb();
				LaserModel();
		 		~LaserModel();
				LaserModel(mapgrid * * );
};
#endif /*LASERMODEL_H_*/
