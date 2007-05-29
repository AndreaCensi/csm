function I0 = ld_fisher0(ld)
% This function computes Fisher's information matrix, in robot coordinates.
% Uses field 'true_alpha' (and 'theta', 'readings').  
% 
% For details about the Fisher's information matrix for localization,
% please see this paper: http://purl.org/censi/2006/accuracy

I0 = zeros(3,3);

for i=1:ld.nrays
	alpha_i = ld.true_alpha(i);
	r = ld.readings(i);

	if isnan(alpha_i) | isnan(r) 
		continue;
	end
	
	phi_i   = ld.theta(i);
	beta_i  = alpha_i - (phi_i);
	
	% dr_dt = v(beta_i)' / cos(beta_i);
	dr_dt = (v(phi_i) + v(phi_i+pi/2) * tan(beta_i))';
	dr_dt = v(alpha_i)' / cos(beta_i);
	dr_dtheta = r * tan(beta_i);
	
	I0(1:2,1:2) = I0(1:2,1:2) + (dr_dt' * dr_dt);
	I0(1:2,3) = I0(1:2,3) + (dr_dt' * dr_dtheta);
	I0(3,1:2) = I0(3,1:2) + (dr_dt * dr_dtheta);
	I0(3,3) = I0(3,3) + dr_dtheta * dr_dtheta;
end

function res = v(a)
	res = [cos(a); sin(a)];
