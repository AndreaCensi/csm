function analyze_run(res, one_scan)
	true_e = join(collect(res, 'true_e'));
	bias = mean(true_e,2)
	e_b = true_e - repmat(bias, 1, size(true_e,2));
	cov_e = cov(e_b')
	
	m.datasets = {};
	m.datasets{1}.points = true_e;
	m.datasets{1}.color = 'r.';

	m.covariances{1}.mean = bias;
	m.covariances{1}.cov = cov_e;
	m.covariances{1}.color = 'r-';
	
	fim = ld_fisher0(one_scan);
	crb = inv(fim) * (0.01^2);
	
	m.covariances{2}.mean = [0;0;0];
	m.covariances{2}.cov =  crb;
	m.covariances{2}.color = 'b--';

	m.covariances{3}.mean = [0;0;0];
	m.covariances{3}.cov =  2 * crb;
	m.covariances{3}.color = 'g--';

	m.covariances{4}.mean = [0;0;0];
	m.covariances{4}.cov =  res{1}.cov_x;
	m.covariances{4}.color = 'm--';

	m.legend = {  ...
	        'Errors ';...
	        'Errors covariance';...
            'Cramer-Rao bound'; 'SM C.-R. bound'; 'estimate of cov' };

	for i=1:size(m.covariances, 2)
		C = m.covariances{i}.cov;
		name = m.legend{i + 1};
		p = display_cov(C);
		fprintf('%20s  %5.2f mm %5.2f mm %5.3f deg   %2.2f %2.2f %2.2f \n', name, ...
			1000*p(1), 1000*p(2), rad2deg(p(3)), p(4), p(5), p(6));
	end

	m.format = '-depsc2';
	m.prefix = 'test1';
	m.extension = 'eps';
	m.title = 'Residual error - ';
	fs = plot_xyt(m);