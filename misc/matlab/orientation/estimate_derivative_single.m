function [fd0,fd0_var] = estimate_derivative_single(t,f,fvar,fd_guess, t0,f0,fvar0,fd_guess0,curv)
	% number of observations
	n = size(t,2);
	% 2*n+1 noises
	R = zeros(n,2*n+1);
	Z = zeros(2*n+1,2*n+1);
	Y = zeros(n,1);
	L = zeros(n,1);
	
	for i=1:n
		
		d = t0-t(i);
		
		% stima derivata seconda
		f2 = (fd_guess0-fd_guess(i)) / d; 
		Y(i, 1) = (f0-f(i)) / d + d.^2 * f2 / 2;
		L(i, 1) = 1;
		
		% rumore con i-esimo
		R(i, i) = 1 / d;
		Z(i, i) = fvar(i);
		% rumore con me
		R(i, 2*n +1 ) = 1 / d;
		% rumore con curvatura
		R(i, i+n) = d.^2 / 2;
		Z(i+n, i+n) = curv;
	end
	
	Z(2*n+1,2*n+1) = fvar0;

	% rumore 
	qR = R*Z*R';
	iR = inv(qR);

	fd0  = inv(L' * iR * L) * L' * iR * Y;
	fd0_var = inv(L' * iR * L);
	
