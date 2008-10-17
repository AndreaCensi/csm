function index = sampleIndex(prepared)
	n = size(prepared,2);
	i = min(max(1, floor(rand*(n-1))), n);
	index = prepared(i);
