#ifndef H_MEASUREMENTSLIKELIHOOD
#define H_MEASUREMENTSLIKELIHOOD

#define L1 1
#define L2 2

#include <csm/csm_all.h>
#include <vector>



class MeasurementsLikelihood
{
public:
//constructors
	MeasurementsLikelihood();
	MeasurementsLikelihood(int likelihood_function, int measurements_number);
	virtual ~MeasurementsLikelihood(void);	
	//class variables
public:
	int function_type;
	double error;
	std::vector<double> grad;
	std::vector<std::vector<double> > hess;
	//methods
public:
	void ComputeAlphaLikelihoods(std::vector<double> x_vector, std::vector<double> alphas0, std::vector<double> alphas_covs);
	void ComputeRangeLikelihoods(std::vector<double> x_vector, std::vector<double> ranges0, std::vector<double> ranges_covs);

};

#endif
