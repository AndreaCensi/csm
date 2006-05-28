function test_icp_covariance1
	% This test executes the icp ``at 0'' for N times and computes the variance

	N = 200; 
	sigma = 0.01;

	step = [0.1;0.1;0]

	params = ts_square([5], [0;2;deg2rad(15)], step);
	params.firstGuess = [0.095;0.095;deg2rad(.5)];

	for i=1:N
		fprintf('=== trial #%d ===\n', i);
		paramsi = params;
		paramsi.laser_sens = ld_add_noise(paramsi.laser_sens, sigma);
		paramsi.laser_ref = ld_add_noise(paramsi.laser_ref, sigma);
		res{i} = icp(paramsi);
	end
	
	test_icp_covariance2_result = res;
	save 'test_icp_covariance2_result.mat' test_icp_covariance2_result
