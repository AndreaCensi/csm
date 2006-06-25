function ld = ld_square(L, pose, rays, fov)
%   ld = ld_square(L, pose, rays, fov)
%   L: length of square side

	square = [-1 -1; 1 -1; 1 1; -1 1; -1 -1]' * L;

	ld = ray_tracing( pose, fov, rays, 'countour_straight', {square},0.00001);
	




