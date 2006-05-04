function [p, alpha] = countour_circle(params, tau)
	radius = params{1};
	
	
	theta = 2 * pi * tau;
	
	p = radius * [cos(theta);sin(theta)];

	alpha = theta+pi;	
	

