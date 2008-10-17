function res = test_bounds_square(theta)

side = 5;
cell=0.1;
x=-side*0.4:cell:side*0.4;
y=-side*0.4:cell:side*0.4;

fov = pi;
rays = 91;

for i=1:size(x,2)
for j=1:size(y,2)

	pose = [x(i); y(j); theta]
	ld = ld_square(side, pose, rays, fov);
	bounds = compute_bounds(ld);
	
	etmax(i,j) = max(sqrt(eig(bounds.C0(1:2,1:2))));
	etmin(i,j) = min(sqrt(eig(bounds.C0(1:2,1:2))));
	eth(i,j) = rad2deg(sqrt(bounds.C0(3,3)));
	
	sigma = 0.01;
	[etmax(i,j) * sigma etmin(i,j) * sigma eth(i,j) * sigma]
	
	data{i,j}.bounds = bounds;
	data{i,j}.ld = ld;
end
end

%res.bounds = bounds;
res.etmax = etmax;
res.etmin = etmin;
res.eth = eth;
res.data = data;

