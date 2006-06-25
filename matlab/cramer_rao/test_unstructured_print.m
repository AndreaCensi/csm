function test_unstructured_print
	% prints the test environment
	
	rho = 5;
	amp = 0.2;
	N   = 30;
	
	nrays = 181;
	fov = pi;

	ld2 = ld_sine(rho, amp, N, [0;0;pi/2], nrays, fov);
	
	f=figure;hold on;
	params.color = 'b-';
	ld_plot(ld2, params);
	
	ld = ld_add_noise(ld2, 0.01);
	params.color = 'r.';
	ld_plot(ld, params)
	axis('equal');
