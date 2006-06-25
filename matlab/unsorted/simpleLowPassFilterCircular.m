% Simple low-pass filter with a gaussian mask, for a circular buffer
function b = simpleLowPassFilterCircular(a, sigma)
	extended=[a a a];
	extendedb=simpleLowPassFilter(extended,sigma);
	b = extendedb(size(a,2)+1:end-size(a,2));
	b = normalize(b);

