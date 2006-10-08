function ld = ld_alloc(min_max_theta, readings)
	% Fills default fields of a LaserData struct
	ld.nrays    = size(readings,2);
	
	
	ld.min_theta = min_max_theta(1);
	ld.max_theta = min_max_theta(2);	
	for i=1:ld.nrays
		ld.theta(i) = ld.min_theta + (i-1) / (ld.nrays-1) * (ld.max_theta-ld.min_theta)
	end

	for i=1:ld.nrays
		if isnan(readings(i))
			ld.valid(i) = 0;
			ld.readings(i) = nan;
		else
			ld.valid(i) = 1;
			ld.readings(i) = readings(i);
		end
	end
	
	@nrays      = nrays
	@valid       = [false  ] * nrays
	@readings    = [GSL::NAN] * nrays
	@theta       = [GSL::NAN] * nrays
	@alpha_valid = [false  ] * nrays
	@alpha       = [GSL::NAN] * nrays
	@cov_alpha   = [GSL::NAN] * nrays
	@cluster     = [-1] * nrays
	
	@odometry    = Vector[GSL::NAN,GSL::NAN,GSL::NAN].col
	@estimate    = Vector[GSL::NAN,GSL::NAN,GSL::NAN].col
	@p = []; @corr = [];
	for i in 0..nrays-1
		@p[i] = Vector[GSL::NAN,GSL::NAN].col
		@corr[i] = Correspondence.new;
		@corr[i].valid = false;
		@corr[i].j1 = -1;
		@corr[i].j2 = -1;
	end
	
	# jump tables
	@up_bigger = [nil]*nrays
	@up_smaller = [nil]*nrays
	@down_bigger = [nil]*nrays
	@down_smaller = [nil]*nrays
	
