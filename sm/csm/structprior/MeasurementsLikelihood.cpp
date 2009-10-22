#include "MeasurementsLikelihood.h"

MeasurementsLikelihood::MeasurementsLikelihood(int likelihood_function)
{
	function_type = likelihood_function;


}

MeasurementsLikelihood::~MeasurementsLikelihood()
{


}

void MeasurementsLikelihood::ComputeLikelihoods(std::vector<double> x_vector)
{
	int n = x_vector.size();
	if (function_type == L2)
	{
		for (int i=0;i<n;i++)
		{
			//double error = 
		
		}
		
	
	}
	
	if (function_type == L1)
	{
	
	
	}


}
