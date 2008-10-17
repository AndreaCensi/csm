function ld = ld_sine(rho, amplitude, periods, pose, rays, fov)
%   ld = ld_sine(rho, amplitude, periods, pose, rays, fov)

	ld = ray_tracing( pose, fov, rays, 'countour_sine', {rho, amplitude, periods},0.00001);
	




