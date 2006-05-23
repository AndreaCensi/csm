function [p, alpha] = countour_circle(params, tau)
	radius = params{1};
	
	if size(radius,2) == 2
		x_radius = radius(1);
		y_radius = radius(2);
	else
		x_radius = radius;
		y_radius = radius;
	end
	
	theta = 2 * pi * tau;
	
	p = [x_radius*cos(theta); y_radius*sin(theta)];

	alpha = atan2(x_radius*sin(theta),y_radius*cos(theta));
	

