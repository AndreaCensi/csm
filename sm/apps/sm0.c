#include "../sm.h"
#include "../laser_data.h"

int main(int argc, const char*argv[]) {
	FILE * file;
	if(argc==1) {
		file = stdin;
	} else {
		const char * filename = argv[1];
		file = fopen(filename,"r");
		if(file==NULL) {
			fprintf(stderr, "Could not open '%s'.\n", filename); 
			return -1;
		}
	}
	
	struct sm_params params;
	struct sm_result result;
	
	params.maxAngularCorrectionDeg = 40;
	params.maxLinearCorrection = 0.5;
	params.maxIterations = 30;
	params.epsilon_xy = 0.001;
	params.epsilon_theta = 0.001;
	params.maxCorrespondenceDist = 2;
	params.sigma = 0.01;
	params.restart = 0;
	params.clusteringThreshold = 0.05;
	params.orientationNeighbourhood = 3;
	params.doAlphaTest = 0;
	params.outliers_maxPerc = 0.85;
	params.doVisibilityTest = 1;

	if(ld_read_next_laser_carmen(file, &params.laser_ref)) {
		printf("Could not read first scan.\n");
		return -1;
	}
	
	while(!ld_read_next_laser_carmen(file, &params.laser_sens)) {
		params.odometry[0] = 0; 
		params.odometry[1] = 0; 
		params.odometry[2] = 0; 
		
	 	sm_icp(&params,&result);
	 
		ld_free(&(params.laser_ref));
		params.laser_ref = params.laser_sens;
	}
	return 0;
}