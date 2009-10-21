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

double Constraint::ApplyConstraint(int* indices, double* params)
{
	double err = 0;
	
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

	return err;
}
