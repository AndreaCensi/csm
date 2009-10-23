#include "ConstraintManager.h"


ConstraintManager::ConstraintManager()
{
	sm_debug("ConstraintManager should take parameters");

}

ConstraintManager::ConstraintManager(std::vector<int> constraint_types)
{
	equal_to_either_num = 10;
	lock_diff_threshold = deg2rad(5);
	
	constraint_types_to_apply = constraint_types;

}

ConstraintManager::~ConstraintManager()
{
	for (int i=0; i<constraints.size();i++)
	{
		delete constraints[i];
	}
	constraints.clear();

}

void ConstraintManager::ClearConstraints()
{
	for (int i=0; i<constraints.size();i++)
	{
		delete constraints[i];
	}
	constraints.clear();

}

void ConstraintManager::ApplyConstraintsAlphas(std::vector<double> x_vector)
{
	//add as many types of constraints as wished
	bool equal_to_either_active = true;
	bool apply_equal_to_either = false;
	bool lock_diff_active = true;
	bool apply_lock_diff = false;
	
	int n = x_vector.size();
	
	//***************************************************************************
	// select the types of constraints we want to apply
	for (int i=0;i< constraint_types_to_apply.size();i++)
	{
		if (equal_to_either_active && constraint_types_to_apply[i] == EQUAL_TO_EITHER)
		{
			apply_equal_to_either = true;
			equal_to_either_active = false;
			continue;
		}
		
		if (lock_diff_active && constraint_types_to_apply[i] == LOCK_DIFF)
		{
			apply_lock_diff = true;
			lock_diff_active = false;
			continue;
		}
		
	}
	//***************************************************************************
	if(apply_equal_to_either)		//add equal_to_either constraints
	{
		//this all could be substituted by a function ApplyEqualToEither
		for (int i=equal_to_either_num-1;i<n-equal_to_either_num;i++)
		{
			
			int mb = min (i-1, n-i-1);
			mb = min (mb, equal_to_either_num-1);
			for (int j=0; j< mb;j++)				//constraints with no consecutive points
			{
				Constraint* c = new Constraint(EQUAL_TO_EITHER);
				double* alpha_values;
				alpha_values[0]= x_vector[i-j]; 
				alpha_values[1]= x_vector[i]; 
				alpha_values[2]= x_vector[i+j];
				Values v = c->ApplyConstraint(alpha_values);
				constraints.push_back(c);							
				e += v.error;
				//update grad
				//update hess?
				
			}
			
		}

	}
	//***************************************************************************
	if(apply_lock_diff)				//add lock_diff constraints
	{
		for (int i=0;i<n-1;i++)
		{
			int* ind;
			ind[0] = i; ind[1]=i+1;
			double* alpha_values;
			alpha_values[0]= x_vector[i]; 
			alpha_values[1]= x_vector[i+1];
			double* p;
			p[0] = PI/2; p[1] = lock_diff_threshold;
			Constraint* c1 = new Constraint(LOCK_DIFF);
			Values v = c1->ApplyConstraint(alpha_values,p);
			constraints.push_back(c1);							
			e += v.error;
			p[0] = -PI/2;
			Constraint* c2 = new Constraint(LOCK_DIFF);
			v = c2->ApplyConstraint(alpha_values,p);
			constraints.push_back(c2);							
			e += v.error;
			//update grad
			//update hess?

		}
	}
	
	

}
