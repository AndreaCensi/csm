function ts = ts_flower(r1, r2, v, pose, step)


	pose1 = pose;
	pose2 = rtcat(pose1,step);
	
	ts.laser_ref  = ray_tracing( pose1, pi, 181, 'countour_flower', {r1,r2,v});
	ts.laser_sens = ray_tracing( pose2, pi, 181, 'countour_flower', {r1,r2,v});


