
function [from, to] = icp_possible_interval(P, ld, maxAngularCorrectionDeg, maxLinearCorrection)
	debug = 0;
	
	delta = abs(deg2rad(maxAngularCorrectionDeg)) + ...
		        abs(atan(maxLinearCorrection/norm(P)));
	
	n = ld.nrays;
	fov = abs(ld.theta(n)-ld.theta(1));
	angleRes = fov/n;
	start_theta = atan2(P(2),P(1));
	start_cell  = ((start_theta - ld.theta(1)) / fov) * n;
	range = ceil(delta/angleRes);

	from = min(n, max( floor(start_cell-range),1));
	to = max(1, min(ceil(start_cell+range),n));

	if debug
		fprintf('start_theta: %f delta: %f start_cell: %d from: %d to: %d\n',...
			rad2deg(start_theta), rad2deg(delta), start_cell, from, to);
		pause(0.001);
	end
