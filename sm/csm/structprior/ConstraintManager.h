#ifndef H_CONSTRAINTMANAGER
#define H_CONSTRAINTMANAGER

#include <vector>

#include "Constraint.h"


class ConstraintManager
{
public:
//constructors
	ConstraintManager(std::vector<int> constraint_types);
	virtual ~ConstraintManager(void);

//class variables
	double e;
	std::vector<Constraint*> constraints;
protected:
	std::vector<int> constraint_types_to_apply;
	int equal_to_either_num;
	double lock_diff_threshold;
	
//methods
public:
	void ApplyConstraintsAlphas(std::vector<double> x_vector);
	void ClearConstraints();
	
protected:
	void ApplyEqualToEither();
	void ApplyLockDiff();


	
	
	
	
};

#endif

