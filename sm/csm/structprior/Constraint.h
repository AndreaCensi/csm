#ifndef H_CONSTRAINT
#define H_CONSTRAINT

#define EQUAL_TO_EITHER 1
#define LOCK_DIFF 2

#define PI 3.14159 

#include <csm/csm_all.h>


class Constraint
{
public:
//constructors
	Constraint(int t);
	virtual ~Constraint(void);
//class variables
	double e;
	double grad;
protected:
	int type;
// class methods
public:
	double ApplyConstraint(int* indices, double* params = NULL);
	void SetType(int t);
};

#endif
