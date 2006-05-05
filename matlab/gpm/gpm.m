function res = gpm(params)
% params.laser_ref
% params.laser_sens
% params.maxAngularCorrectionDeg
% params.maxLinearCorrection
% params.emXYsigma

	if not(isfield(params.laser_ref, 'alpha_valid'))
		fprintf('Computing surface normals for ld1.\n');
		params.laser_ref = computeSurfaceNormals_sound(params.laser_ref);
	end
	
	if not(isfield(params.laser_sens, 'alpha_valid'))
		fprintf('Computing surface normals for ld2.\n');
		params.laser_sens = computeSurfaceNormals_sound(params.laser_sens);
	end
	
	

	% GHT o(n^2) algorithm
	% This loop can be much more efficient ( o(n) )
	k=1;
	for a=find(params.laser_ref.alpha_valid)
		p = params.laser_ref.points(:,a);
		delta = abs(deg2rad(params.maxAngularCorrectionDeg)); 
		delta = delta + abs(atan(params.maxLinearCorrection/norm(p)));
		
		angleRes = pi / size(params.laser_sens.points,2);
		range = ceil(delta/angleRes);
		from = a-range;
		to = a+range;
		from = max(from, 1);
		to   = min(to, size(params.laser_sens.points,2));
			
	for b=from:to
	
		if params.laser_sens.alpha_valid(b)==0
			continue;
		end
	
		phi = params.laser_ref.alpha(a) - params.laser_sens.alpha(b);
		phi = normAngle(phi);
		
		if abs(phi) > deg2rad(params.maxAngularCorrectionDeg)
			continue
		end
		
		T = params.laser_ref.points(:,a) - rot(params.laser_sens.points(:,b), phi);
		

		% Surface normal
		alpha = params.laser_ref.alpha(a);
		
		if norm(T) > params.maxLinearCorrection
			continue
		end
		
		if false % Let T slide along alpha
			T = vers(alpha) * (vers(alpha)'*T);
		end
		
		
		
		weight=1;
		weight = weight * sqrt(params.laser_ref.alpha_error(a));
		weight = weight * sqrt(params.laser_sens.alpha_error(b));
		weight=sqrt(weight);
		res.corr{k}.T = T;
		res.corr{k}.phi = phi; 
		res.corr{k}.alpha = alpha; 
		res.corr{k}.weight = 1/weight; 
		k=k+1;
	end
	end
	
	% number of correspondences
	N = size(res.corr,2);
	% build L matrix (Nx2) 
	L = zeros(N,2); L2 = zeros(2*N,3);
	Y = zeros(N,1); Y2 = zeros(2*N,1);
	W = zeros(N,1); W2 = zeros(2*N,1);
	Phi = zeros(N,1);
	samples = zeros(3,N);
	for k=1:N
		L(k,:) = vers(res.corr{k}.alpha)';
		Y(k,1) = vers(res.corr{k}.alpha)' * res.corr{k}.T;
		W(k,1) = res.corr{k}.weight;
		Phi(k,1) = res.corr{k}.phi;
		block = [vers(res.corr{k}.alpha)' 0; 0 0 1];
		L2((k-1)*2+1:(k-1)*2+2,1:3) = block;
		Y2((k-1)*2+1:(k-1)*2+2,1) = [Y(k,1); res.corr{k}.phi]; 
		W2((k-1)*2+1:(k-1)*2+2,1) = [res.corr{k}.weight;res.corr{k}.weight];
		
		samples(:,k) = [res.corr{k}.T; res.corr{k}.phi];
	end
	
%	theta = hill_climbing(Phi, W, deg2rad(20), mean(Phi), 20);
%	fprintf('Theta: %f\n', rad2deg(theta));
	
	
%	X = [0.3;0;deg2rad(15)];
%	X = [0;0;theta];
	X = mean(samples,2);	
	for it=1:params.maxIterations
		fprintf(strcat(' X: ',pv(X),'\n'))
		Sigma = diag([0.5 0.5 deg2rad(40)].^2);% / ((1+it)/2);
	
		% update weights
		uW = zeros(N,1);
		uW2 = zeros(2*N,1);
		for k=1:N
			myX = [res.corr{k}.T; res.corr{k}.phi];
			weight = W(k,1) * mynormpdf( myX-X, [0;0;0], Sigma);
			
			uW(k,1)  = weight;
			uW2((k-1)*2+1:(k-1)*2+2,1) = [weight;weight];
		end;
		
		Ri2 = diag(uW2);
		Xhat = inv(L2'*Ri2 * L2) * L2' * Ri2 * Y2;
	%	fprintf(strcat(' Xhat: ',pv(X),'\n'))
		X = Xhat;
		%X(3) = theta;
		pause(0.1)
	end

	
	res.X = X;
	res.Phi = Phi;
	res.W = W;
	res.samples = samples;
	
%res.weight = normalize01(res.weight) ; 

%function b = normalize01(a)
%	b = (a-min(a))/(max(a)-min(a));
	
function res = rot(v, phi)
	res = [cos(phi) -sin(phi); sin(phi) cos(phi)] * v;
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';

function q = sq(v)
	q=v*v;
	
function p = mynormpdf(x, mu, sigma);
    mahal = (x-mu)' * inv(sigma) * (x-mu);    
	 p = (1 / (sqrt(2*pi) * det(sigma))) * exp(-0.5*mahal);
	 
function res = hill_climbing(x, weight, sigma, x0, iterations)
% hill_climbing(x, weight, sigma, x0, iterations)
	for i=1:iterations
		for j=1:size(x)
			updated_weight(j) =  weight(j) * mynormpdf(x(j), x0, sigma^2);
			updated_weight(j) =  weight(j) * exp( -abs(x(j)- x0)/ (sigma));
			
		end
		x0 = sum(x .* updated_weight') / sum(updated_weight);
		%fprintf(' - %f ', rad2deg(x0));
	end
	%fprintf('\n');
	res = x0;


