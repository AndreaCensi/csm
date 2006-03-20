% Simple low-pass filter with a gaussian mask (bidimensional version)
function b = simpleLowPassFilter2(a, sigma)
	amp=2*sigma+1; 
	mask=zeros(amp,amp);
	
	for i=1:amp
	for j=1:amp
		mask(i,j) = exp(-((i-sigma-1)^2+(j-sigma-1)^2)/sigma^2);
	end
	end
	
	b = conv2(a,mask,'same');

