% Normalizes a vector in the [0,1] range
function b = normalize(a)
	minimum = min(a);
	maximum = max(a);
	b = (a - minimum) / (maximum-minimum);

