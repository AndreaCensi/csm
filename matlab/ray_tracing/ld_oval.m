function ld = ld_circle(x_radius, y_radius, pose, rays, fov)
%   ld = ld_circle(x_radius, y_radius, pose, rays, fov)
%   Oval at (0,0)

	ld = ray_tracing( pose, fov, rays, 'countour_circle', {[x_radius y_radius]},0.001);
	




