function ts = ts_square(L, pose, step)
%   ts = ts_square(L, pose, step)
%   L: length of square side or if size(L)=2 then it is [Lx Ly]
%   pose2 = pose (+) step

	if size(L) == 1
		M = L;
	else
		M = diag(L);
	end
	
	square = M * [-1 -1; 1 -1; 1 1; -1 1; -1 -1]';
	
	pose1 = pose;
	pose2 = rtcat(pose1,step);
	
	ts.laser_ref  = ray_tracing( pose1, pi, 181, 'countour_straight', {square},0.00001);
	ts.laser_sens = ray_tracing( pose2, pi, 181, 'countour_straight', {square},0.00001);
	




