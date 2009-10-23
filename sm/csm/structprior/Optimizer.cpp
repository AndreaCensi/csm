#include "Optimizer.h"

Optimizer::Optimizer(LDP ld, MeasurementsLikelihood ml, ConstraintManager cm)
{
	laser_data = ld;
	measurements_likelihood = ml;
	constraint_manager = cm;

}

Optimizer::~Optimizer(void)
{

}

void Optimizer::OptimizeAlphas()
{

}
