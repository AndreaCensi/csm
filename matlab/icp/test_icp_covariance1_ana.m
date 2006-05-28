function an = test_icp_covariance1_ana(res, test_title, prefix)
	% analyzes result of the test_icp_covariance1 or test_icp_covariance2
	
	truth =[0.1;0.1;0];
	
	for i=1:size(res,2)
		p(:,i) = res{i}.X - truth;
	end
	
	
	an.bias = mean(p,2);
	an.sample_cov = cov(p');
	
	an.p = p;
	
	crb = compute_bounds(res{1}.laser_ref);
	
	sigma = 0.01;
	an.crb = crb.C0 * (sigma^2);
	%an.estimated_cov = res{1}.Cov * (sigma^2);
	
	m.datasets{1}.points = p;
	m.datasets{1}.color = 'r.';
	m.covariances{1}.mean = an.bias;
	m.covariances{1}.cov = an.sample_cov;
	m.covariances{1}.color = 'r-';
	m.covariances{2}.mean = [0;0;0];
	m.covariances{2}.cov = an.crb;
	m.covariances{2}.color = 'b--';
	m.legend = {'Samples'; 'Sample \Sigma'; 'Cramer-Rao'; 'Location'; 'BestOutside'};
	m.format = '-depsc2';
	m.prefix = prefix;
	m.extension = 'eps';
	m.title = test_title;
	fs = plot_xyt(m);


