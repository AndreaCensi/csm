function lds = convertFile(file)
Output = readFileInCells(file);

k=1;

for i=1:size(Output)
	line = Output(i,:);
	if 1==strcmp(line(1), 'FLASER')
		lds{k} = convertLine(line);
		k = k + 1;
	end
end
