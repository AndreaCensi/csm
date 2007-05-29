function res = collect(cells, field)
	res = {};
	for i=1:size(cells,2) 
		s = cells{i};
		res{i} = s.(field);
	end