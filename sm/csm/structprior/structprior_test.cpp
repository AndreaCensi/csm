#include <stdio.h>
#include <csm/csm_all.h>

#include "ConstraintManager.h"
#include "MeasurementsLikelihood.h"


int main(int argc, const char** argv) 
{

	if (argc < 2)
	{
		sm_error("Provide input file's name as an argument\n");
		return -1;
	}

	const char* file_name = argv[1];
	FILE* input_file = fopen(file_name, "r");
	

	LDP laserdata;
	if(!(laserdata = ld_read_smart(input_file))) {
		sm_error("Could not read scan.\n");
		return -1;
	}
	
	
	// the types of constraints we want to apply (i.e. depending on the environment...)
	std::vector<int> cons_types;
	cons_types.push_back(EQUAL_TO_EITHER);
	//cons_types.push_back(LOCK_DIFF);

	ConstraintManager cons_manager(cons_types);
	
	MeasurementsLikelihood f(L2);

	
	//minimizer.Minimize(laserdata,f,cons_manager...)...



	return 0;
	
}
