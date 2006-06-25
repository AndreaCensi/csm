function test_unstructured(res)
	% analyzes result of the test_unstructured
	
	rho = 5;
	amp = 0.2;
	N   = 30;
	
	nrays = 181;
	fov = pi;

	ld2 = ld_sine(rho, amp, N, [0;0;0], nrays, fov);
	
	crb = compute_bounds(ld2);
	
	eig(crb.I0)
	
	% C of the f function in ld_sine
	C = amp^2 * N^2 / (2 * rho^2);
	D = amp^2 * N^4 / rho^4;
	fprintf('Real information matrix:\n');
	crb.I0
	fprintf('Approximated information matrix:\n');
	I = cramer_rao_approximation(ld2, rho, C)
	C
	
	
	truth =[0;0;0];
	
	for i=1:size(res,2)
		points(:,i) = res{i}.X - truth;
	end
	

	bias = mean(points,2);
	sample_cov = cov(points');
	
	sigma = 0.01;
	
	if false
	
	m.datasets{1}.points = points;
	m.datasets{1}.color = 'r.';
	m.covariances{1}.mean = bias;
	m.covariances{1}.cov = sample_cov;
	m.covariances{1}.color = 'r-';
	m.covariances{2}.mean = [0;0;0];
	m.covariances{2}.cov = crb.C0 * (sigma^2);
	m.covariances{2}.color = 'b--';
	m.covariances{3}.mean = [0;0;0];
	m.covariances{3}.cov = inv(I) * (sigma^2);
	m.covariances{3}.color = 'k-.';
	m.legend = {'Samples'; 'Sample \Sigma'; 'Cramer-Rao'; ...
		'Approximated\newline Cramer-Rao'};
	m.format = '-depsc2';
	m.prefix = 'icp_un';
	m.extension = 'eps';
	m.title = 'ICP, Unstructured env. -';
	fs = plot_xyt(m);
	end

	e_th = points(3,:);
	
	real_std_th = sqrt(var(e_th'));
	crb_std_th = sqrt(inv(crb.I0(3,3)))*sigma;
	
	batta = ((sigma/rho)^2 / (nrays * C)) * (1 + sigma * D / (2* C^2));
	approx_crb_std_th =  sigma*sqrt(inv(nrays*(rho^2)*C))
	
	batta_std_th = sqrt(batta);
	
	fprintf('Sample std: %f\n', rad2deg(real_std_th));
	fprintf('Cramer-Rao std: %f\n', rad2deg(crb_std_th));
	fprintf('Cramer-Rao (approx) std: %f\n', rad2deg(approx_crb_std_th));
	fprintf('Battacharya std: %f\n', rad2deg(batta_std_th));
	
	ib=0;
	n = 1;
	
	for theta=ld2.theta
		s = rho * theta;
		f = f(amp, N, rho, s);
		f1 = f_dot(amp, N, rho, s);
		f2 = f_ddot(amp, N, rho, s);
	%	ib = ib + (nf_dot^2) / (sigma^2) * inv(1+((sigma^2)/2)*( (nf_ddot^2)/(nf_dot^4)));
	
		inf = 2*(f1^6) / ( ((sigma/rho)^2)*(2*f1^4+(f2*sigma)^2));
	
		if (f1^2) < 0.001
		n = n+1;
			continue
		end
		
		inf = (f1*rho/sigma)^2 * inv(1+0.5*(f2*sigma/(f1^2))^2);
		ib = ib +  inf;
		
		m(1,n)=f1; m(2,n)=f2; m(3,n)=inf; 
		m(4,n)=f; m(5,n)=ld2.readings(n);
		n = n+1;
	end
	
	mb = inv(ib);
	figure
	subplot(4,1,1); plot(m(1,:));
	subplot(4,1,2); plot(m(2,:));
	subplot(4,1,3); plot(m(3,:));
	subplot(4,1,4);
		hold on; plot(m(4,:),'r-');
		plot(m(5,:),'b.');
	
	fprintf('Battacharya real: %f\n', rad2deg(sqrt(mb)));
	
function res = f(amp,N,rho,s)
	res = rho+amp * cos( (N/rho) * s);
	
function res = f_dot(amp, N, rho, s)
	res = -amp * (N/rho) * sin ( (N/rho) * s);
function res = f_ddot(amp, N, rho, s)
	res = -amp * ((N/rho)^2) * cos ( (N/rho) * s);

function I = cramer_rao_approximation(ld, rho, C)
	fov =  max(ld.theta)-min(ld.theta);
	
	if (abs(fov - pi) < deg2rad(1))
		I_xy_th = [0; C*rho*(ld.nrays/pi)*2];
	else
		if abs(fov - 2*pi) < deg2rad(1)
		I_xy_th = [0; 0];
		else
		error(sprintf('detected fov: %f', rad2deg(fov)));
		end
	end
	lambda_I_t = (ld.nrays/2) * (1+C);
	I_th =  ld.nrays*(rho^2)*C;
		

	I_t = [lambda_I_t 0; ...
	     0  lambda_I_t];
		  
	I = [I_t I_xy_th; ...
			I_xy_th' I_th];
