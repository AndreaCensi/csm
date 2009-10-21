#include "Constraint.h"


Constraint::Constraint(int t)
{
	type = t;
}

Constraint::~Constraint(void)
{

}

void Constraint::SetType(int t)
{
	type = t;
}

Values Constraint::ApplyConstraint(double alphas[], double params[])
{
	Values v;
	double err = 0;
	double grd = 0;
	
	switch(type)
	{
		case EQUAL_TO_EITHER:
		{
			
			
			break;
		}
		case LOCK_DIFF:
		{
			
			break;
		}
	
	
	}

	v.error = err;
	v.grad = grd;

	return v;
}
