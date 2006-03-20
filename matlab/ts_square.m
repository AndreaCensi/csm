function ts = ts_square(L, pose, step)

	square = [-1 -1; 1 -1; 1 1; -1 1]' * L;

	pose1 = pose;
	pose2 = rtcat(pose1,step);
	
	ts.laser_ref  = ray_tracing( pose1, pi, 181, 'countour_straight', {square});
	ts.laser_sens = ray_tracing( pose2, pi, 181, 'countour_straight', {square});
	




