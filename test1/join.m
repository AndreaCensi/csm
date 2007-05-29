function res = join(cells)
	columns = size(cells,2);
	rows = size(cells{1},1);
	res = zeros(rows, columns);
	for j=1:columns
		res(1:rows,j) = cells{j};
	end
