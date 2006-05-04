function res = test_bounds_square2_tmp(stuff)

for i=1:size(stuff,1)
for j=1:size(stuff,2)

	ld = stuff{i,j}.ld;
	bounds = compute_bounds(ld);
	
	etmax(i,j) = max(sqrt(eig(bounds.C0(1:2,1:2))));
	etmin(i,j) = min(sqrt(eig(bounds.C0(1:2,1:2))));
	eth(i,j) = rad2deg(sqrt(bounds.C0(3,3)));
	
	data{i,j}.bounds = bounds;
	data{i,j}.ld = ld;
	
	if rand>0.95
		pause(0.01)
	end
end
fprintf('%d\n', i);
end

%res.bounds = bounds;
res.etmax = etmax;
res.etmin = etmin;
res.eth = eth;
res.data = data;

