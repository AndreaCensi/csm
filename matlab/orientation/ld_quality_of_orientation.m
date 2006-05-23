function ld_quality_of_orientation(ld)

	e = rad2deg(normAngle(ld.alpha-ld.true_alpha));
	
	fprintf('Bias: %f deg \n', mean(e));
	fprintf('Deviation: %f deg \n', sqrt(var(e)));
	
	
	fprintf('Mean error: %f deg \n', rad2deg(sqrt(max(ld.alpha_error))));
	
