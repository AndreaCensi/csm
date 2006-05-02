function ts = ts_flower(r1, r2, v, pose, step)
% ts = ts_flower(r1,r2,v,pose,step)
%
% returns ts.laser_ref and ts.laser_sens
%
% r1,r2,v give the flower shape
% pose2 = pose (+) step

	pose1 = pose;
	pose2 = rtcat(pose1,step);
	
	ts.laser_ref  = ray_tracing( pose1, pi, 181, 'countour_flower', {r1,r2,v},0.000001);
	ts.laser_sens = ray_tracing( pose2, pi, 181, 'countour_flower', {r1,r2,v},0.000001);


