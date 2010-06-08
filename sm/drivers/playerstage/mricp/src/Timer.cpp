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

#include "Timer.h"
#include <ctime>
#include <cstddef>

MricpTimer::MricpTimer()
{
	gettimeofday(&start_time,NULL);
}
double MricpTimer::TimeElapsed() // return in usec
{
	gettimeofday(&end_time,NULL);
	time_diff = ((double) end_time.tv_sec*1000000 + (double)end_time.tv_usec) -
	            ((double) start_time.tv_sec*1000000 + (double)start_time.tv_usec);
	return  time_diff;
}
MricpTimer::~MricpTimer()
{
}
void MricpTimer::Reset()
{
	gettimeofday(&start_time,NULL);
}
void MricpTimer::Synch(double period)
{
	struct timespec ts;
	int us;

	double time_elapsed = this->TimeElapsed();
	if (time_elapsed < (period*1000))
	{
		us = static_cast<int>(period*1000-time_elapsed);
		ts.tv_sec = us/1000000;
		ts.tv_nsec = (us%1000000)*1000;
		nanosleep(&ts, NULL);
	}
}

