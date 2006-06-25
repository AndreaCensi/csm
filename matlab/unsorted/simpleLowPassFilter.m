% Simple low-pass filter with a gaussian mask
function b = simpleLowPassFilter(a, sigma)
	amp=2*sigma; filter=exp(-((-amp:amp).^2)/sigma^2);
	b = normalize(convn(a,filter,'same'));

