function [p, alpha] = countour_flower(params, tau)
	r1 = params{1};
	r2 = params{2};
	v  = params{3};
	
	theta = -pi + 2 * pi * tau;
	rho = r1 + r2 * cos(theta*v);

	p = rho * [cos(theta);sin(theta)];

	alpha = nan;	
	

