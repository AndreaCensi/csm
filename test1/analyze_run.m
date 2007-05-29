function analyze_run(res)
	res;
	x = join(collect(res, 'x'));
	m = mean(x,2)
	e = x - repmat(m, 1, size(x,2));
	cov_e = cov(e')
	display_cov(cov_e)
	