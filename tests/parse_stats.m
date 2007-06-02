function res = parse_stats(filename)
	res.data = load(filename);
	
	res.total_error = res.data(:,2)';
	res.valid = res.data(:,3)';
	res.mean_error = res.data(:,4)';
	res.iterations = res.data(:,5)';
	res.time = res.data(:,6)';
	res.x = res.data(:,7:9)';
	res.u = res.data(:,10:12)';
	
	
	res.e = res.u-res.x;
	
	for j=1:size(res.e,2)
		res.e(3,j) = normAngle(res.e(3	,j));
	end
	
	
function b = normAngle(a)
	b = atan2(sin(a),cos(a));