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
	double* grd;
	int size = 3;
	
	switch(type)
	{
		//*********************************************************************************
		case EQUAL_TO_EITHER:
		{
			size = 3;
			double e1 = 0.5*(alphas[1]-alphas[0])*(alphas[1]-alphas[0]);
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
		//*********************************************************************************
		case LOCK_DIFF:
		{
			size = 2;
			double bias = params[0]; double threshold = params[1];
			if (abs( alphas[1] - (alphas[0]+bias) ) < threshold)
			{
        		e = 0.5*( alphas[1] - (alphas[0]+bias) );
        	}
        	else
        	{
        	
        	}
			
			break;
		}
		//*********************************************************************************
		
		default:
			sm_debug("Unrecognized type of constraint \n");
	
	}

	v.error = err;
	for (int i=0;i<size;i++)
		v.grad[i] = grd[i];

	return v;
}
