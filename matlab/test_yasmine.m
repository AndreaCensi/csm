function res = test_yasmine(k)

sigma=0.00001;

	S=load('logs/bighouse_half.mat');
	log = S.log_bighouse_half;

	params.laser_ref = log{k};
	params.laser_ref = add_noise(params.laser_ref, sigma);
	params.laser_sens = log{k};
	params.laser_sens = add_noise(params.laser_sens, sigma);
	params.maxAngularCorrectionDeg = 15;
	params.maxLinearCorrection = 0.5;
	params.sigma = sigma;
	res.params = params;

	tic
	result = yasmine2(params);
	toc
	
	res.result = result;
		
		
function res = add_noise(ld, sigma) 
	res = ld;
	res.readings = res.readings + sigma * randn(1,res.nrays);
	res.points = [res.readings .* cos(res.theta);  res.readings .* sin(res.theta)]; 
	res = computeSurfaceNormals_sound(res);
