% Requires: rtcat, ld_square, ld_plot, ld_add_noise, file_exists
% Requires: sample_normal, icp, compute_bounds, plot_xyt
function aarl_square2(position)
	% This test executes the icp ``at 0'' for N times and computes the variance

	%% Number  of trials.
	N = 100; 
	%% Noise for sensor..
	sigma = 0.01;

	%% First pose.
	position = [-2;3;deg2rad(30)];
	%% Step.
	truth = [0;0;deg2rad(0.4)];
	%% Second pose.
	position2 = rtcat(position, truth);
	%% Covariance of the first guess.
	guess_cov = diag([0.02^2 0.02^2 deg2rad(0.5)^2]);
	%% Side of the triangle..
	L = 10;

	params.laser_ref  = ld_square(L, position, 361, 2*pi);
	params.laser_sens = ld_square(L, position2, 181, 2*pi);
	params.laser_sens1 = ld_square(L, position, 181, 2*pi);

	if false
	figure; hold on;
	ld_plot(params.laser_ref);
	c.color = 'b.';
	ld_plot(params.laser_sens,c);
	c.color = 'g.';
	ld_plot(ld_add_noise(params.laser_sens,sigma),c);
	hold off;
	end
	
	paramsi.maxAngularCorrectionDeg = 5;
	paramsi.maxLinearCorrection = 0.2;
	
	filename = 'icpcov_square2-results.mat';
	if file_exists(filename)
		load(filename);
	else
		for i=1:N
			guess = truth + sample_normal([0;0;0], guess_cov, 1);

			fprintf('=== Loc trial #%d ===\n', i);
			paramsi.laser_ref = params.laser_ref;
			paramsi.laser_sens = ld_add_noise(params.laser_sens, sigma);
			paramsi.firstGuess = guess;
			paramsi.do_covariance = false;
			aarl_square_loc{i} = icp(paramsi);
	
			fprintf('=== SM trial #%d ===\n', i);
			paramsi.laser_ref = ld_add_noise(params.laser_sens1, sigma);
			paramsi.firstGuess = guess ;
			aarl_square_sm{i} = icp(paramsi);
		end
	
		save(filename, 'aarl_square_loc','aarl_square_sm');
	end
	
	if true
			guess = truth; % + sample_normal([0;0;0], guess_cov, 1);
			fprintf('=== SM trial 1 ===\n');
			paramsi.laser_sens = ld_add_noise(params.laser_sens, sigma);
			paramsi.laser_ref = ld_add_noise(params.laser_sens, sigma);
			paramsi.firstGuess = guess ;
			paramsi.maxIterations = 1;
			paramsi.do_covariance = true;
			one = icp(paramsi);
			one
	end
	
	for i=1:size(aarl_square_loc,2)
		res_loc(:,i) = aarl_square_loc{i}.X - truth;
		 res_sm(:,i) =  aarl_square_sm{i}.X - truth;
	end
	

	bias_loc = mean(res_loc,2)
	bias_sm = mean(res_sm,2)
	cov_loc = cov(res_loc')
	cov_sm  = cov(res_sm')
	
	b = compute_bounds(params.laser_sens);
	cov_cr = sigma^2 * inv(b.I0)

	
	fprintf('Cramer rao:\n');
	print_covariance('cov_cr',cov_cr);
	fprintf('Localization: mean = %s \n', pv(bias_loc));
	print_covariance('cov_loc',cov_loc);
	fprintf('Scan matching: mean = %s \n', pv(bias_sm));
	print_covariance('cov_sm',cov_sm);
	eig(cov_sm)
	fprintf('Estimated \n');
	print_covariance('cov_sm',one.Cov);
	eig(one.Cov)
	
	if true
		m.datasets = {};
		m.datasets{1}.points = res_loc;
		m.datasets{1}.color = 'r.';
		m.datasets{2}.points = res_sm;
		m.datasets{2}.color = 'g.';
		m.covariances{1}.mean =  bias_loc;
		m.covariances{1}.cov = cov_loc;
		m.covariances{1}.color = 'r-';
		m.covariances{2}.mean = [0;0;0];
		m.covariances{2}.cov =  cov_cr;
		m.covariances{2}.color = 'b--';
		m.covariances{3}.mean =  bias_sm;
		m.covariances{3}.cov = cov_sm;
		m.covariances{3}.color = 'g:';
		m.covariances{4}.mean =  bias_sm;
		m.covariances{4}.cov = one.Cov;
		m.covariances{4}.color = 'm-';
		m.legend = {  ...
			'Loc. cov.';...
			'C.-R. bound';'S.M. cov.';'estimated'};
	%	m.legend = { 'Loc. error'; ...
	%		'Loc. cov.';'S/M cov.';...
	%		'C.-R. bound'};
		m.format = '-depsc2';
		m.prefix = 'aarl_square2';
		m.extension = 'eps';
		m.title = 'Residual error - ';
		fs = plot_xyt(m);
	end

function print_covariance(name, M)
	fprintf(' %s\n', name);
	M

