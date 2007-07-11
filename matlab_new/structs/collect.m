function res = collect(cells, field)
	res = {};
	k = 1;
	for i=1:size(cells,2) 
		s = cells{i};
		if isfield(s, field)
			res{k} = s.(field);
		else
			ref{k} = nan;
		end
		k = k + 1;
	end