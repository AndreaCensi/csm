function [p, alpha] = countour_sine(params, tau)
	% params = {rho, amplitude, periods_in_circle}

	rho = params{1};
	amplitude = params{2};
	periods = params{3};
	
	theta = 2 * pi * tau;
	
	p = sine(theta,rho,amplitude, periods);
	
	epsilon = 0.001;
	p1 = sine(theta-epsilon,rho,amplitude, periods);
	p2 = sine(theta+epsilon,rho,amplitude, periods);
	
	v_alpha = rot(pi/2)*(p2-p1);
	alpha = atan2(v_alpha(2),v_alpha(1));
	

function p = sine(theta, rho, amplitude, periods)
	s = theta * rho;
	dist = rho + amplitude * cos(s * periods/rho );
	p = dist * [cos(theta);sin(theta)];

