
params.maxAngularCorrectionDeg = 25; % degrees
params.maxLinearCorrection = 0.15; % meters
params.emXYsigma = 0.2;

		i=3;
	params.laserData1 = log_bighouse(i-1);
	params.laserData2 = log_bighouse(i);

	fprintf('executing gpm\n');
	%res = gpm(params); 

	%fprintf('sampling\n');
	%r = gpmSample(res, 500);
	
	
	% Densità
	d = zeros(1,360);
	for m=1:size(res.weight,2)
		th = ceil(mod(rad2deg(res.alpha(m)),360));
		d(th) = d(th) + res.weight(m);
	end
	
	d2 = simpleLowPassFilterCircular(d, 30);
	%d = hist( mod(rad2deg(res.alpha),360), 1:360);
	%mask = exp(-d2);
	
	for m=1:size(res.weight,2)
		k = ceil( mod(rad2deg(res.alpha(m)), 360));
		
		w(m) = res.weight(m) / d2(k  );
	end
	
	
	% Densità
	d3 = zeros(1,360);
	for m=1:size(res.weight,2)
		th = ceil(mod(rad2deg(res.alpha(m)),360));
		d3(th) = d3(th) + w(m);
	end
