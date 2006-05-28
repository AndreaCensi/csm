function res = test_icp_unstructured1
	% This test executes the icp ``at 0'' for N times and computes the variance

	rho = 5;
	amp = 0.2;
	N   = 30;
	
	map_rays = 720;
	nrays = 181;
	fov = pi;

	laser_ref  = ld_sine(rho, amp, N, [0;0;0], map_rays, fov);
	laser_sens = ld_sine(rho, amp, N, [0;0;0], nrays, fov);
	
	
	N = 100; 
	sigma = 0.01;


	for i=1:N
		fprintf('=== trial #%d ===\n', i);
		paramsi.laser_ref = laser_ref;
		paramsi.laser_sens = ld_add_noise(laser_sens, sigma);
		paramsi.firstGuess = [0.005;-0.005;deg2rad(0.5)];
		res{i} = icp(paramsi);
	end
	
	test_icp_unstructured1_result = res;
	save 'test_icp_unstructured1_result.mat' test_icp_unstructured1_result
