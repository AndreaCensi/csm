#include "ConstraintManager.h"


ConstraintManager::ConstraintManager(LDP ld, std::vector<int> constraint_types)
{
	equal_to_either_num = 10;
	lock_diff_threshold = deg2rad(5);
	
	laser_data = ld;
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

void ConstraintManager::ApplyConstraints()
{
	//add as many types of constraints as wished
	bool equal_to_either_active = false;
	bool apply_equal_to_either = false;
	bool lock_diff_active = false;
	bool apply_lock_diff = false;
	
	int n = laser_data->nrays;
	
	for (int i=0;i< constraint_types_to_apply.size();i++)
	{
		if (equal_to_either_active && constraint_types_to_apply[i] == EQUAL_TO_EITHER)
		{
			apply_equal_to_either = true;
			continue;
		}
		
		if (lock_diff_active && constraint_types_to_apply[i] == LOCK_DIFF)
		{
			apply_lock_diff = true;
			continue;
		}
		
	}
	
	if(apply_equal_to_either)		//add equal_to_either constraints
	{
		
		for (int i=equal_to_either_num-1;i<n-equal_to_either_num;i++)
		{
			
			int mb = min (i-1, n-i-1);
			mb = min (mb, equal_to_either_num-1);
			for (int j=0; j< mb;j++)
			{
				Constraint* c = new Constraint(EQUAL_TO_EITHER);
				int* ind;
				ind[0] = i-j; ind[1] =i; ind[2] =i+j;							
				e += c->ApplyConstraint(ind);
				constraints.push_back(c);
			}
			
		}

	}
	
	if(apply_lock_diff)				//add lock_diff constraints
	{
		Constraint* c = new Constraint(LOCK_DIFF);
		for (int i=0;i<n-1;i++)
		{
			int* ind;
			ind[0] = i; ind[1]=i+1;
			double* p;
			p[0] = PI/2; p[1] = lock_diff_threshold;
			constraints.push_back(c);
			e+= c->ApplyConstraint(ind,p);
			p[0] = -PI/2;
			e+= c->ApplyConstraint(ind,p);
			constraints.push_back(c);
	
		}
	}
	
	

}
