
function point = transform(point, dx)
% rotate then translate point
	point = dx(1:2,1) + rot(dx(3)) * point;

