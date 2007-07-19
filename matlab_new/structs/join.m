function res = join(cells)
	columns = size(cells,2);
	rows = size(cells{1},1);
	res = zeros(rows, columns);
	for j=1:columns
		if size(cells{j},1) > 0
			res(1:rows,j) = cells{j};
		else
			fprintf('No value for cell %d\n', j);
			pause(0.01);
			res(1:rows,j) = zeros(rows,1) * NaN;
		end
	end
