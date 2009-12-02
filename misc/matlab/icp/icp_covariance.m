
function res = icp_covariance(params, current_estimate, P, valids, jindexes)
	% res = icp_covariance(params, current_estimate, P, valids, jindexes)
	%
	% Compute the covariance of the ICP estimate.
	%
	% params.laser_sens  laser data structure describing sensor scan 
	% params.laser_ref   laser data structure describing reference scan
	%
	%   Fields used:
	%     laser_*.nrays    number of rays
	%     laser_*.points   2xnrays vector of readings in cartesian coordinates
	%
	% params.sigma       sigma of noise on readings
	% current_estimate   solution found by ICP: pose of laser_sens in laser_ref reference frame
	% valids             nrays x 1 vector: valids(i) == 1 if the i-th point in laser_sens has valid correspondences
	% jindexes           nrays x 2  correspondences:  jindexes(i,1), jindexes(i,2) are the indices
	%                    of the closest points in laser_ref of the i-th point in laser_sens
	% 
	%  P                 unused (probably legacy from some old code)
	%  
	k=1; 
	
	Etot = 0;
	Gtot = [0;0;0];
	G2tot = zeros(3,3);
	MMtot = zeros(3,3);
	
	x=current_estimate(1);
	y=current_estimate(2);
	theta=current_estimate(3);
	
	%% These are the interval to approximate the derivatives with 
	%% the increments.
	eps_xy = 0.000001;
	eps_t = deg2rad(0.01);
	
	center = zeros(3);
	
	dgE_di = zeros(3, params.laser_sens.nrays);
	dgE_dj = zeros(3, params.laser_ref.nrays);
	
	% let's iterate on the "valid" points of laser_sens
	for a=find(valids)
		% index of point in laser_sens
		i = a;
		% indices of the two corresponding points in laser_ref
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
		
		%% The following is ugly but conceptually simple:
		%% we define an error function E_k and then we derive it numerically.
		
		%% Note that we use the Matlab lambda-notation 
		%%    func = @(x) x^2
		%% who would have thought that also Matlab can be Lispy? :-)
		
		%% The error function is the traditional point-to-segment distance (squared)
		E_k = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			norm(...
				transform(v_i*rho_i_, [x_;y_;theta_])  ... 
				- ...
				closest_point_on_segment(v_j1*rho_j1_, v_j2*rho_j2_, ...
					transform(v_i*rho_i_, [x_;y_;theta_])) ...
			)^(2);
				%% first point p_i transformed by current estimate
				%% and the closest point on the segment
	
		%% First derivative
		gradEk = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			[ deriv(@(xx)E_k(rho_i_,rho_j1_,rho_j2_,xx,y_,theta_), x_, eps_xy); ...
			  deriv(@(yy)E_k(rho_i_,rho_j1_,rho_j2_,x_,yy,theta_), y_, eps_xy); ...
			  deriv(@(tt)E_k(rho_i_,rho_j1_,rho_j2_,x_,y_,tt), theta_, eps_t)];
			
		%% Second derivative
		d2Ek_dx2 = @(rho_i_, rho_j1_, rho_j2_, x_, y_, theta_) ...
			[ deriv(@(xx)gradEk(rho_i_,rho_j1_,rho_j2_,xx,y_,theta_), x_, eps_xy) ...
			  deriv(@(yy)gradEk(rho_i_,rho_j1_,rho_j2_,x_,yy,theta_), y_, eps_xy) ...
			  deriv(@(tt)gradEk(rho_i_,rho_j1_,rho_j2_,x_,y_,tt), theta_, eps_t)];

		%% The noise has three components: epsilon_i, epsilon_j1, epsilon_j2
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
		
		Mk = [eye(2) (rot(theta+pi/2)*p_i)] ;
		MMtot = MMtot + Mk' * Mk;
		
		k=k+1;
	end
	                                                
	dA_dz = inv(G2tot) * [dgE_di dgE_dj];
	
	%Etot
	%Gtot
	%G2tot

	
	sigma = params.sigma;
	% dgE_di
	% dgE_dj
	% dA_dz
	
	fprintf('icp_covariance: Using sigma: %f', sigma);
	
	R = sigma^2 * eye(params.laser_ref.nrays+params.laser_sens.nrays);
	
	res.sm_cov_censi = dA_dz * R * dA_dz';
	
	s2 = Etot / (k-3);
	res.sm_cov_bengtsson = 2 * s2 * inv(G2tot);
	
	
	dA_dz1 = inv(G2tot) * [dgE_di ];
	R1 = sigma^2 * eye(params.laser_sens.nrays);
	res.loc_cov_censi = dA_dz1 * R1 * dA_dz1';
	
	res.sm_cov_bengtsson_improved =  s2 * inv( MMtot );
	
	fprintf('Bengtsson, improved:');
	res.sm_cov_bengtsson_improved 
	fprintf('Bengtsson, original:')
	res.sm_cov_bengtsson 
	
	%res
