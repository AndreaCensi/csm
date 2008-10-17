function ld_quality_of_orientation(ld)

	e = rad2deg(normAngle(ld.alpha-ld.true_alpha));
	
	for i=1:size(e,2)
		if abs(e(1)) > 10
			e(i) = 0;
		end
	end
	
	fprintf('Bias: %f deg \n', mean(e));
	fprintf('Deviation: %f deg \n', sqrt(var(e)));
	
	
	fprintf('Mean error: %f deg \n', rad2deg(sqrt(max(ld.alpha_error))));
	
	
	figure
	plot(e,'r.')
	title('errors (degree)');
