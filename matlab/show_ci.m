function show_ci(matrices)

	cim = ci(matrices);

	figure; hold on;

	for i=1:size(matrices,2)
		plotGauss2Db([0;0], matrices{i}, 'k');
	end

	plotGauss2Db([0;0], cim, 'r');
	
