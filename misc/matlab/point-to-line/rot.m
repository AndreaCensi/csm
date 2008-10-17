function r=rot(theta) 
% rot(theta) rotation matrix of order two
	r = [cos(theta) -sin(theta); sin(theta) cos(theta)];
