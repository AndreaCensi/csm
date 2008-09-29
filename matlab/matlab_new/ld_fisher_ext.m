function I0 = ld_fisher_ext(ld, vars)
% This is an extension of function ld_fisher0 which takes into account
% different variances for each ray. It also does not count rays
% for which ld.valid(i) is set to 0.
%
% This function computes Fisher's information matrix, in robot coordinates.
% Uses field 'true_alpha' (and 'theta', 'readings').  
% If true_alpha is NAN, it tries to use alpha.
% 
% For details about the Fisher's information matrix for localization,
% please see this paper: http://purl.org/censi/2006/accuracy

I0 = zeros(3,3);

nused = 0;
for i=1:ld.nrays
	alpha_i = NaN;
	if isfield(ld, 'true_alpha')
		alpha_i = ld.true_alpha(i);
	end
	if isnan(alpha_i)
		alpha_i = ld.alpha(i);
	end
	
	r = ld.readings(i);

	if isnan(alpha_i) | isnan(r) | not(ld.valid(i))
		continue;
	end
	
	phi_i   = ld.theta(i);
	beta_i  = alpha_i - (phi_i);
	
	dr_dt = v(alpha_i)' / cos(beta_i);
	dr_dtheta = r * tan(beta_i);
	
	mult = 1 / vars(i);
	I0(1:2,1:2) = I0(1:2,1:2) + mult * (dr_dt' * dr_dt);
	I0(1:2,3) = I0(1:2,3) +  mult * (dr_dt' * dr_dtheta);
	I0(3,1:2) = I0(3,1:2) +  mult *(dr_dt * dr_dtheta);
	I0(3,3) = I0(3,3) +  mult * (dr_dtheta * dr_dtheta);
	
	nused = nused +1;
end
fprintf('FIM computed using %d rays.\n', nused);

function res = v(a)
	res = [cos(a); sin(a)];
