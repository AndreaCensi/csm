#include "icp.h"

void icp(struct icp_input*input,struct icp_output*output) {
	
	struct pose x_old;
	
	x_old = input->odometry;
	int iteration;
	for(iteration=0; iteration<input->maxIterations;iteration++) {
		
		
		
	}
}