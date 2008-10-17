function res = test_yasmine3(k)
	% systematic experiments

	x_true = [1;0;deg2rad(-10.5)];
	ts0 = ts_square(8, [1;-2;deg2rad(10)], x_true );
%	ts0 = ts_flower(8, 2, 10, [0;0;0], x_true);
	sigma=0.005;
	odometry_cov = diag( [0.2 0.2 deg2rad(5)].^2 );

	odometry_cov = diag( [0.5 0.5 deg2rad(5)].^2 );

	N = 50;
	
	for i=1:N
		ts{i} = ts0;
		
		ts{i}.input.laser_ref = add_noise(ts0.laser_ref, sigma);
		ts{i}.input.laser_sens = add_noise(ts0.laser_sens, sigma);
		
		ts{i}.input.maxAngularCorrectionDeg = 40;
		ts{i}.input.maxLinearCorrection = 2;
		ts{i}.input.sigma = sigma;
		
		ts{i}.input.trim = 0.05;
		ts{i}.input.chi_perc = 0.95;
		ts{i}.input.chi_limit = 50;
		ts{i}.input.odometry = sample_normal(x_true, odometry_cov, 1);
		ts{i}.input.odometry_cov = odometry_cov;

		tic
		ts{i}.output = yasmine2(ts{i}.input);
		toc
			
	end

	res = ts;
	
function res = add_noise(ld, sigma) 
	res = ld;
	res.readings = res.readings + sigma * randn(1,res.nrays);
	res.points = [res.readings .* cos(res.theta);  res.readings .* sin(res.theta)]; 
	res = computeSurfaceNormals_sound(res);
