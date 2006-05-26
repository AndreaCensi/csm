function an = test_icp_covariance1_ana(res)
	% analyzes result of the test
	
	truth =[0.1;0.1;0];
	
	for i=1:size(res,2)
		p(:,i) = res{i}.X - truth;
	end
	
	
	mean(p,2)
	cov(p')
	
	
	an.bias = mean(p,2);
	an.sample_cov = cov(p');
	
	an.p = p;
	
	crb = compute_bounds(res{1}.laser_ref);
	
	sigma = 0.01;
	an.crb = crb.C0 * (sigma^2);
	an.estimated_cov = res{1}.Cov * (sigma^2);
	
	f = figure; hold on;
	
		plot(p(1,:),p(2,:),'r.');
		plotGauss2Db(an.bias(1:2),an.sample_cov(1:2,1:2),'r-');
		plotGauss2Db([0;0],an.crb(1:2,1:2),'b-');
		plotGauss2Db([0;0],an.estimated_cov(1:2,1:2),'g-');

		legend('Samples', 'Sample \Sigma', 'Crb', 'estimated \Sigma');
		
		axis('equal');
		title('errors on x, y');
		
	f = figure; hold on;
		plot(p(1,:),p(3,:),'r.');
		plotGauss2Db(an.bias(1:2:3),an.sample_cov(1:2:3,1:2:3),'r-');
		plotGauss2Db([0;0],an.crb(1:2:3,1:2:3),'b-');
		plotGauss2Db([0;0],an.estimated_cov(1:2:3,1:2:3),'g-');
		legend('Samples', 'Sample \Sigma', 'Crb', 'estimated \Sigma');
		axis('equal');
		title('errors on x, \phi');
		
	f = figure; hold on;
		plot(p(2,:),p(3,:),'r.');
		plotGauss2Db(an.bias(2:3),an.sample_cov(2:3,2:3),'r-');
		plotGauss2Db([0;0],an.crb(2:3,2:3),'b-');
		plotGauss2Db([0;0],an.estimated_cov(2:3,2:3),'g-');
		legend('Samples', 'Sample \Sigma', 'Crb', 'estimated \Sigma');
		axis('equal');
		title('errors on y, \phi');
		
	an.sample_cov
	mycov(p)
	
function res = mycov(P)

	res = zeros(size(P,1));
	for i=1:size(P,2)
		res = res + P(:,i) * P(:,i)';
	end
	res = res / (1-size(P,2));
