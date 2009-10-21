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
	double grd[3] = {0,0,0};
	
	switch(type)
	{
		case EQUAL_TO_EITHER:
		{
			double e1 = 0.5*(alphas[1]-alphas[0]);
			double e2 = 0.5*(alphas[2]-alphas[1]); 
	
			if (e1 < e2)
			{
				 err = e1;

			}
				  
			else
			{
				 err = e2;
			}

			
			break;
		}
		case LOCK_DIFF:
		{
			
			break;
		}
	
	
	}

	v.error = err;
	for (int i=0;i< 3;i++)
		v.grad[i] = grd[i];

	return v;
}
