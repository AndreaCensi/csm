
function res = icp_covariance(params, current_estimate, P, valids,jindexes)
	k=1; 
	
	Etot = 0;
	Gtot = [0;0;0];
	G2tot = eye(3);
	
	x=current_estimate(1);
	y=current_estimate(2);
	theta=current_estimate(3);
	
	eps_xy = 0.000001;
	eps_t = deg2rad(0.01);
	
	center = zeros(3);
	
	dgE_di = zeros(3, params.laser_sens.nrays);
	dgE_dj = zeros(3, params.laser_ref.nrays);
	
	for a=find(valids)
		i = a;
		j1 = jindexes(i,1);
		j2 = jindexes(i,2);
		
		p_i  = params.laser_sens.points(:,a);
		p_j1 = params.laser_ref.points(:,j1);
		p_j2 = params.laser_ref.points(:,j2);
		
		rho_i = norm(p_i);
		rho_j1 = norm(p_j1);
		rho_j2 = norm(p_j2);
		v_i = p_i / norm(p_i);
		v_j1 = p_j1 / norm(p_j1);
		v_j2 = p_j2 / norm(p_j2);
		
		E_k = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			norm(...
				transform(v_i*rho_i_, [x_;y_;theta_])  ... 
				- ...
				closest_point_on_segment(v_j1*rho_j1_, v_j2*rho_j2_, ...
					transform(v_i*rho_i_, [x_;y_;theta_])) ...
			)^2;
	
		gradEk = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			[ deriv(@(xx)E_k(rho_i_,rho_j1_,rho_j2_,xx,y_,theta_), x_, eps_xy); ...
			  deriv(@(yy)E_k(rho_i_,rho_j1_,rho_j2_,x_,yy,theta_), y_, eps_xy); ...
			  deriv(@(tt)E_k(rho_i_,rho_j1_,rho_j2_,x_,y_,tt), theta_, eps_t)];
			
		d2Ek_dx2 = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			[ deriv(@(xx)gradEk(rho_i_,rho_j1_,rho_j2_,xx,y_,theta_), x_, eps_xy) ...
			  deriv(@(yy)gradEk(rho_i_,rho_j1_,rho_j2_,x_,yy,theta_), y_, eps_xy) ...
			  deriv(@(tt)gradEk(rho_i_,rho_j1_,rho_j2_,x_,y_,tt), theta_, eps_t)];

		d2Ek_dxdei = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			deriv(@(ei)gradEk(ei,rho_j1_,rho_j2_,x_,y_,theta_), rho_i_, eps_xy);
	
		d2Ek_dxdej1 = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			deriv(@(ej1)gradEk(rho_i_,ej1,rho_j2_,x_,y_,theta_), rho_j1_, eps_xy);
	
		d2Ek_dxdej2 = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			deriv(@(ej2)gradEk(rho_i_,rho_j1_,ej2,x_,y_,theta_), rho_j2_, eps_xy);
			
		Etot = Etot + E_k(rho_i, rho_j1, rho_j2, x,y,theta);
		
		Gtot = Gtot+ gradEk(rho_i,  rho_j1, rho_j2, x,y,theta);
		G2tot = G2tot+ d2Ek_dx2(rho_i,  rho_j1, rho_j2, x,y,theta);
		
		di = d2Ek_dxdei(rho_i,  rho_j1, rho_j2, x,y,theta);
		dj1 = d2Ek_dxdej1(rho_i,  rho_j1, rho_j2, x,y,theta);
		dj2 = d2Ek_dxdej2(rho_i,  rho_j1, rho_j2, x,y,theta);
	
		dgE_di(:,i) = dgE_di(:,i) + di;
		dgE_dj(:,j1) = dgE_dj(:,j1) + dj1;
		dgE_dj(:,j2) = dgE_dj(:,j2) + dj2;
		
		k=k+1;
	end
	
	dA_dz = inv(G2tot) * [dgE_di dgE_dj];
	
	Etot
	Gtot
	G2tot
	
	sigma = 0.01;
	dgE_di
	dgE_dj
	dA_dz
	
	R = sigma^2 * eye(params.laser_ref.nrays+params.laser_sens.nrays);
	
	
	res = dA_dz * R * dA_dz';
