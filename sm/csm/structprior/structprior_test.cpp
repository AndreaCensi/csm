#include <stdio.h>
#include <csm/csm_all.h>

#include "ConstraintManager.h"


int main(int argc, const char** argv) 
{

	if (argc < 2)
	{
		sm_error("Provide input file's name as an argument\n");
		return 0;
	}

	const char* file_name = argv[0];
	FILE* input_file = fopen(file_name, "r");

	LDP laserdata;
	laserdata = ld_read_smart(input_file);
	
	std::vector<int> cons_types;
	cons_types.push_back(EQUAL_TO_EITHER);
	//cons_types.push_back(LOCK_DIFF);

	ConstraintManager cons_manager(laserdata, cons_types);
	cons_manager.ApplyConstraints();
	
	//minimizer.Minimize(x,cons_manager.constraints...)...



	return 1;
	
}
