function res = gpmSample(gpmres,n)

m = size(gpmres.T, 2);
prepared = prepareSampling(1:m, gpmres.weight);
fprintf('Prepared\n');
i=1;

while i<n

	i1 = sampleIndex(prepared);
	i2 = sampleIndex(prepared);
	
	alpha1 = gpmres.alpha(i1);
	alpha2 = gpmres.alpha(i2);
	rho1 = vers(alpha1)' * gpmres.T(:,i1);
	rho2 = vers(alpha2)' * gpmres.T(:,i2);
	
	A = [vers(alpha1)'; vers(alpha2)'];
	b = [rho1; rho2];
	
	if det(A) == 0 
		fprintf('No. det = %f\n', det(A));
		continue;
	end

	W = diag([gpmres.weight(i1) gpmres.weight(i2)]); 
	C = inv(A' * inv(W) * A);
	
	e = eigs(C);
	if min(e) / max(e) < 0.5
		fprintf('No.\n');
		continue
	end
	
	T = A\b
	res(:, i) = T;
	i = i + 1;
	pause(0.0001);
end


function v = vers(theta)
	v = [cos(theta) sin(theta)]';

