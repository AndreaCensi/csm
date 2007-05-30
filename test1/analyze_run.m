function analyze_run(res)
	x = join(collect(res, 'x'));
	bias = mean(x,2)
	e = x - repmat(bias, 1, size(x,2));
	cov_e = cov(e')
	display_cov(cov_e)
	
	m.datasets = {};
	m.datasets{1}.points = x;;
	m.datasets{1}.color = 'r.';

	m.covariances{1}.mean = bias;
	m.covariances{1}.cov = cov_e;
	m.covariances{1}.color = 'r-';
	
	fim = ld_fisher0(scan2);
	crb = inv(fim) * (0.01^2);
	
	m.covariances{2}.mean = bias; %[0;0;0];
	m.covariances{2}.cov =  crb;
	m.covariances{2}.color = 'b--';

	m.covariances{3}.mean = bias; %[0;0;0];
	m.covariances{3}.cov =  2 * crb;
	m.covariances{3}.color = 'g--';

	m.legend = {  ...
	        'Errors ';...
	        'Errors covariance';...
            'Cramer-Rao bound'; 'SM C.-R. bound' };

	m.format = '-depsc2';
	m.prefix = 'test1';
	m.extension = 'eps';
	m.title = 'Residual error - ';
	fs = plot_xyt(m);