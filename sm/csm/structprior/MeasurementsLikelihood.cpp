#include "MeasurementsLikelihood.h"

MeasurementsLikelihood::MeasurementsLikelihood(int likelihood_function, int measurements_number)
{
	function_type = likelihood_function;
	
	error = 0;
	grad.resize(measurements_number);
	hess.resize(measurements_number);
	for (int i=0;i<measurements_number;i++)
		hess[i].resize(measurements_number);

}

MeasurementsLikelihood::~MeasurementsLikelihood()
{

}


void MeasurementsLikelihood::ComputeAlphaLikelihoods(std::vector<double> x_vector, std::vector<double> alphas0, std::vector<double> alphas_covs )
{
	int n = x_vector.size();
	
	error = 0;
	if (grad.size() != 0)
		grad.clear();
	if (hess.size() != 0)
		hess.clear();
		
	// may be redundant, it's added in case some measurements are pre-discarded or something
	grad.resize(n);
	hess.resize(n);
	for (int i=0;i<n;i++)
		hess[i].resize(n);
	
	//****************************************************************************************
	if (function_type == L2)
	{
		
		for (int i=0;i<n;i++)
		{
			double cov_alpha = alphas_covs[i];
			
			double e = 0.5 * (x_vector[i] - alphas0[i]) * (x_vector[i] - alphas0[i]) /cov_alpha;
			double g= - (alphas0[i] - x_vector[i])/ cov_alpha;
			double h = 1/cov_alpha;
			
			error+= e;
			grad[i] = g;
			
			for(int k=0;k<n;k++)
			{
				for (int l=0;l<n;l++)
				{
					if(k==n && l==n)
						hess[k][l] = h;
					else
						hess[k][l] = 0;
										
				}
			}
			
		}
		
	
	}
	//****************************************************************************************
	if (function_type == L1)
	{
	
	
	}
	
}


void MeasurementsLikelihood::ComputeRangeLikelihoods(std::vector<double> x_vector, std::vector<double> ranges0, std::vector<double> ranges_covs)
{

	if (function_type == L2)
	{
	
	
	
	
	
	}
	
	
	
	if (function_type == L1)
	{
	
	
	
	
	
	}
	
	





}










