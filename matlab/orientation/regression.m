
function [theta, rho, sumOfSqError] = regression(points)

	n = size(points,2);
	if n<2
		error('I need at least 2 points');
	end
	
	mu = mean(points,2);
	
	s_x2 = 0;
	s_y2 = 0;
	s_xy = 0;
	
	for i=1:n
		s_x2 = s_x2 + (points(1,i)-mu(1))*(points(1,i)-mu(1));
		s_y2 = s_y2 + (points(2,i)-mu(2))*(points(2,i)-mu(2));
		s_xy = s_xy + (points(1,i)-mu(1))*(points(2,i)-mu(2));
	end
	
	theta = 0.5 * atan2(-2*s_xy, s_y2-s_x2);
	rho = mu(1) * cos(theta) + mu(2) * sin(theta);

	if rho>0
		rho = - rho;
		theta = theta + pi;
	end
	
	% compute error
	sumOfSqError = 0;
	for i=1:n
		% distance to line
		dist = rho - (cos(theta) * points(1,i) + sin(theta) * points(2,i));
		sumOfSqError = sumOfSqError + dist*dist; 
	end
