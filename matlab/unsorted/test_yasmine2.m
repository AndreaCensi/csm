function res = test_yasmine2(k)

	%x_true = [1;-2;deg2rad(-10.5)];
	%x_true = [0.2;0;deg2rad(5.5)];
	%ts = ts_square(8, [1;-4;0], x_true);
	%ts = ts_flower(4, 1, 10, [0;0;0], x_true);
	
	x_true = [1;0;0];
	ts = ts_flower(4, .5, 10, [0;0;0], x_true);
	sigma=0.005;
	odometry_cov = diag( [0.2 0.2 deg2rad(5)].^2 );

	ts.laser_ref = add_noise(ts.laser_ref, sigma);
	ts.laser_sens = add_noise(ts.laser_sens, sigma);
	ts.laser_sens.estimate = [0;0;0];
	ts.laser_sens.odometry = [0;0;0];
	
	
	ts.maxAngularCorrectionDeg = 40;
	ts.maxLinearCorrection = 2;
	ts.sigma = sigma;

	ts.odometry = sample_normal(x_true, odometry_cov, 1)
	ts.odometry_cov = odometry_cov;

	ts.trim = 0.05;
	ts.chi_perc = 0.95;
	ts.chi_limit = 20;

	tic
	result = yasmine2(ts);
	toc
	
	res.result = result;
	res.ts = ts;
	
	e = res.result.x_hat - x_true
		
	f = figure;
		params.plotNormals = true;	
		params.rototranslated = true;

		params.color = 'b.';
		plotLaserData(ts.laser_ref, params);
		params.color = 'r.';
		ld2 = ts.laser_sens;
		ld2.estimate = rtcat(ts.laser_ref.estimate, result.x_hat);
		plotLaserData(ld2, params);
		
	axis('equal');

function res = add_noise(ld, sigma) 
	res = ld;
	res.readings = res.readings + sigma * randn(1,res.nrays);
	res.points = [res.readings .* cos(res.theta);  res.readings .* sin(res.theta)]; 
	res = computeSurfaceNormals_sound(res);
