function [f1_guess, f1_var] = estimate_derivative(t, f0, f0cov, curv, f1guess, interval)

left=interval; right=interval;

N = size(f0,2);
% for each point
for k=1:N
	start = max(k-left,1);
	stop   = min(k+right,N);
	
	% number of observations
	n = stop-start+1 -1;
	% 2*n+1 noises
	R = zeros(n,2*n+1);
	Z = zeros(2*n+1,2*n+1);
	Y = zeros(n,1);
	L = zeros(n,1);
	
	index=1;
	for i=start:1:stop
		if i==k
			continue
		end
		
		d = t(k)-t(i);
		
		% stima derivata seconda
		f2 = (f1guess(k)-f1guess(i)) / d; 
		Y(index, 1) = (f0(k)-f0(i)) / d + d.^2 * f2 / 2;
		L(index, 1) = 1;
		
		% rumore con i-esimo
		R(index, index) = 1 / d;
		Z(index, index) = f0cov(i);
		% rumore con me
		R(index, 2*n +1 ) = 1 / d;
		% rumore con curvatura
		R(index, index+n) = d.^2 / 2;
		Z(index+n, index+n) = curv;
		index = index +1;
	end
	
	Z(2*n+1,2*n+1) = f0cov(k);

	% rumore 
	qR = R*Z*R';
	iR = inv(qR);

	f1_guess(k)  = inv(L' * iR * L) * L' * iR * Y;
	f1_var(k) = inv(L' * iR * L);
	
	if k == 2
		n
		L
		Z
		Y
		R
		qR
		f1_var(k)
		f1_guess(k)
		iR
		f0(start:stop)
	end
end


