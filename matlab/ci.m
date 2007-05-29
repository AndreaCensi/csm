function M = ci(matrices)

	s = zeros(size(matrices{1}));
	
	n = size(matrices,2);
	for i=1:n
		s = s + inv(matrices{i});	
	end
	
	s = s / n;
	M = inv(s);