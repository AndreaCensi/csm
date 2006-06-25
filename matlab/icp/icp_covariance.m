
function res = icp_covariance(params, current_estimate, P, valids)
	k=1; 
	
	theta = 0; t = [0;0];
	
	d2E_dx2 = zeros(3,3);
	centro = zeros(3,3);

	for a=find(valids)
		p_i = transform(params.laser_sens.points(:,a), current_estimate);
		p_j = P(:,a);
		
		rho_i = norm(p_i);
		rho_j = norm(p_j);
		v_i = p_i / rho_i;
		v_j = p_j / rho_j;
		
		d2Ek_dx2 = [ eye(2)  (rho_i * Rdot(theta) * v_i); ...
			(rho_i * Rdot(theta) * v_i)' (rho_i*v_i'*Rddot(theta)'*(t-rho_j*v_j))];
			
		d2E_dx2 = d2E_dx2 + d2Ek_dx2;
	
		d2Ek_dxdzk = [ (rot(theta) * v_i) v_j; ...
		 ((t-rho_j*v_j)'*Rdot(theta)*v_i) (-rho_j*v_j'*Rdot(theta)*v_i)];
		 
		centro = centro + d2Ek_dxdzk*d2Ek_dxdzk';
		
		k=k+1;
	end
	
	res.d2E_dx2 = d2E_dx2;
	res.Cov = inv(d2E_dx2) * centro * inv(d2E_dx2);
	
	
function res = Rdot(theta)
	res = rot(theta+pi/2);

function res = Rddot(theta)
	res = rot(theta+pi); % XXX controlla meglio

