function ld = ld_circle(radius, pose, rays, fov)
%   ld = ld_circle(radius, pose, rays, fov)
%   Circle at (0,0)

	ld = ray_tracing( pose, fov, rays, 'countour_circle', {radius},0.001);
	




