function [alpha,alpha_var] = arras_fit( thetas , rhos , var)

	n = size(thetas,2);
	
	xw=0; yw=0; 
	
	for i=1:n
		w(i) = 1/var;
	end
	
	
	for i=1:n
		
	end

	x = rhos .* cos(thetas);
	y = rhos .* sin(thetas);
	
	xw = sum( w.*x) / sum(w);
	yw = sum( w.*y) / sum(w);
	
	num = -2 * sum((yw-y).*(xw-x).*w);
	den = sum( w.* ( (yw-y).^2 -(xw-x).^2 ) );
	
	alpha = 0.5 * atan(num/den);
	r = xw * cos(alpha)+yw*sin(alpha);
	
	if r<0
		alpha = alpha- pi;
	end
	
	
	alpha_var = 1;
	
	
