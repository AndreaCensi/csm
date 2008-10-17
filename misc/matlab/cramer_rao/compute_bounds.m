function bounds = compute_bounds(ld)
% bounds = compute_bounds(ld)
% ld: laserdata
% return bounds.I0
%        bounds.C0 if I0 is invertible


I0 = zeros(3,3);

for i=1:ld.nrays
	r = ld.readings(i);
	alpha_i = ld.true_alpha_abs(i);

	if isnan(alpha_i) | isnan(r) 
		continue;
	end
	
	phi_i   = ld.theta(i);
	theta   = ld.odometry(3);
	beta_i  = alpha_i - (theta+phi_i);
	
	% dr_dt = v(beta_i)' / cos(beta_i);
	dr_dt = (v(theta+phi_i) + v(theta+phi_i+pi/2) * tan(beta_i))';
	dr_dt = v(alpha_i)' / cos(beta_i);
	dr_dtheta = r * tan(beta_i);
	
	
	I0(1:2,1:2) = I0(1:2,1:2) + (dr_dt' * dr_dt);
	I0(1:2,3) = I0(1:2,3) + (dr_dt' * dr_dtheta);
	I0(3,1:2) = I0(3,1:2) + (dr_dt * dr_dtheta);
	I0(3,3) = I0(3,3) + dr_dtheta * dr_dtheta;
end


bounds.I0 = I0;
bounds.C0 = inv(bounds.I0);


function res = v(a)
	res = [cos(a); sin(a)];
