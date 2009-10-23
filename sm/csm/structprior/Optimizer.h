#ifndef H_OPTIMIZER
#define H_OPTIMIZER

#include <csm/csm_all.h>
#include "MeasurementsLikelihood.h"
#include "ConstraintManager.h"

class Optimizer
{

//constructors
public:
	Optimizer(LDP ld, MeasurementsLikelihood ml, ConstraintManager cm); 
	virtual ~Optimizer(void);
//class variables
	LDP laser_data;
	MeasurementsLikelihood measurements_likelihood;
	ConstraintManager constraint_manager;
//methods
	std::vector<double> OptimizeAlphas();
	void OptimizeRanges();
	void OptimizePoses();

};
#endif
