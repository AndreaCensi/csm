% Requires: rtcat

	%% First pose.
	params.position = [-0.3;0.1;deg2rad(10)];
	%% Step.
	truth_cov = diag([0.35^2 0.35^2 deg2rad(17.5)^2]);
	params.truth = sample_normal([0;0;0], truth_cov, 1);
	%% Covariance of the first guess.
	params.guess_cov = diag([0.35^2 0.35^2 deg2rad(7.5)^2]);
	params.fov = 1.4*pi;
	params.nrays = 180;
	params.sigma = 0.01;
	
	%% Second pose.
	params.position2 = rtcat(params.position, params.truth);

	L = 5;
	seglist = { 0.4*L*[-1.2 1; -1 -1]', L*[1.5 1; 1 -1]', ...
			0.3*L*[1 -1.3; -1 -1]', L*[1.4 1; -1 1]'}
	 
	params.laser_ref  = ...
		ray_tracing_polygon(seglist, params.position, 180, params.fov, inf);
	params.laser_sens = ...
		ray_tracing_polygon(seglist, params.position2, params.nrays, params.fov, inf);

	params.interactive = false;
	
	params.laser_sens = ld_add_noise(params.laser_sens, params.sigma);
	params.laser_ref = ld_add_noise(params.laser_ref, params.sigma);
	
	
	res = icp2(params);	
