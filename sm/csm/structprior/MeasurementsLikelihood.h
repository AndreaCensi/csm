#ifndef H_MEASUREMENTSLIKELIHOOD
#define H_MEASUREMENTSLIKELIHOOD

#define L1 1
#define L2 2

#include <csm/csm_all.h>
#include <vector>

struct LValues{

	double error;
	double grad[];

};

class MeasurementsLikelihood
{
public:
//constructors
	MeasurementsLikelihood(int likelihood_function);
	virtual ~MeasurementsLikelihood(void);	
	//class variables
protected:
	int function_type;
public:
	std::vector<LValues> values_vector;
	//methods
public:
	void ComputeLikelihoods(std::vector<double> x_vector);
	

};

#endif
