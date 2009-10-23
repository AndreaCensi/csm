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

std::vector<double> Optimizer::OptimizeAlphas()
{
	std::vector<double> x;
	if (measurements_likelihood.function_type == L2)
	{
		std::vector<double> alpha0_vector;
		std::vector<double> covs_vector;
		for(int i=0;i<laser_data->nrays;i++)
		{
			alpha0_vector.push_back(laser_data->alpha[i]);
			x.push_back(laser_data->alpha[i]);
			covs_vector.push_back(laser_data->cov_alpha[i]);
		
		}
		
		for (int k=0;k<1000;k++)
		{
			// chek...
			constraint_manager.ApplyConstraintsAlphas(x);
			measurements_likelihood.ComputeAlphaLikelihoods(x,alpha0_vector,covs_vector);
			
			
			
			
		}
	
	
	}
	
	
	if (measurements_likelihood.function_type == L1)
	{
	
	
	}

}

void Optimizer::OptimizeRanges()
{

}

void Optimizer::OptimizePoses()
{


}
