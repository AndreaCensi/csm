function res = yasmine(params)
% params.sigma
% params.laser_sens     % sensor (indexed by i)
% params.laser_ref      % map    (indexed by j)  laser_sens * (x,y,theta) = laser_ref 
% params.maxAngularCorrectionDeg
% params.maxLinearCorrection

	% readings for which a valid alpha was computed
	valid1 = find(params.laser_sens.alpha_valid);
	
	ktot = 1;
	
	num_noises = params.laser_sens.nrays*2 + params.laser_ref.nrays *2;
	num_corr = 50 *params.laser_sens.nrays;
	res.M = zeros(num_corr*2, num_noises);
	res.Y = zeros(num_corr*2, 1);
	res.L = zeros(num_corr*2, 3);
	
	for i=valid1
		
		fprintf('%d / %d \n', i, params.laser_sens.nrays);
		
		theta_i     = params.laser_sens.theta(i);
		    p_i     = params.laser_sens.readings(i) * vers(theta_i);
		alpha_i     = params.laser_sens.alpha(i);
		alpha_i_var = params.laser_sens.alpha_error(i);
		
		% estimate other interval
		delta = abs(deg2rad(params.maxAngularCorrectionDeg)) ...
		       +abs(atan(params.maxLinearCorrection/norm(p_i)));
		angleRes = pi / params.laser_ref.nrays;
		range = ceil(delta/angleRes);
		from = i-range;
		to = i+range;
		from = max(from, 1);
		to   = min(to, params.laser_ref.nrays);
	
		%from = 1;
		%to = params.laser_ref.nrays;
		% putting all correespondences in "corre"
		k = 1;
		corre={};
		for j=from:to
			if params.laser_ref.alpha_valid(j)==0
				continue;
			end
		
			p_j = params.laser_ref.readings(j) * vers(params.laser_ref.theta(j));
			alpha_j = params.laser_ref.alpha(j);
			
			phi_hat = normAngle(alpha_j - alpha_i);
			T_hat = p_j - rot(phi_hat) *  p_i;
			
			if abs(phi_hat) > deg2rad(params.maxAngularCorrectionDeg)
				continue
			end
			if norm(T_hat) > params.maxLinearCorrection
				continue
			end
			
			corre{k}.j = j;
			corre{k}.alpha_j = alpha_j;
			corre{k}.phi_hat = phi_hat;
			corre{k}.T_hat = T_hat;
			k = k+1;
		end
		
		for k=1:size(corre,2)
			% regressore
			j = corre{k}.j ;
			theta_j = params.laser_ref.theta(j);
			alpha_j = params.laser_ref.alpha(j);;
			alpha_j_var = params.laser_ref.alpha_error(j);
			alpha_i_var = params.laser_sens.alpha_error(i);
			phi_hat = corre{k}.phi_hat;
			  T_hat = corre{k}.T_hat;
			proj =  vers(alpha_j)';
			% Lk * (T phi) = yk
			
			Lk = [ proj 0; 0 0 1];
			Yk = [ proj*T_hat; phi_hat];
		
			res.L((ktot-1)*2+1:(ktot-1)*2+2,1:3) = Lk;
			res.Y((ktot-1)*2+1:(ktot-1)*2+2,1)   = Yk;
			
			index_e_i = (i-1)*4 + 1;
			index_w_i = (i-1)*4 + 2;
			index_e_j = (j-1)*4 + 3;
			index_w_j = (j-1)*4 + 4;
			
			v_i = vers(theta_i);
			v_j = vers(theta_j);

			% incertezza proj
			C = sqrt(size(corre,2));
			res.M((ktot-1)*2+1, index_e_i) = C * (- proj * rot(phi_hat) * v_i);
			res.M((ktot-1)*2+1, index_e_j) = C * (  proj                * v_j);
			res.M((ktot-1)*2+1, index_w_i) = C * ( ...
				vers(alpha_j+pi/2)'*T_hat + ...
				proj *  rot(phi_hat+pi/2) * p_i );
			res.M((ktot-1)*2+1, index_w_j) = C * (- proj * rot(phi_hat+pi/2) * p_i);
			% incertezza angolo
			res.M((ktot-1)*2+2, index_w_i) = C * -1;
			res.M((ktot-1)*2+2, index_w_j) = C * 1; 
			
			Rk = [res.M((ktot-1)*2+1, index_e_i) ...
			      res.M((ktot-1)*2+1, index_e_j) ...
					res.M((ktot-1)*2+1, index_w_i) ...
					res.M((ktot-1)*2+1, index_w_j);...
					0 0 ...
					res.M((ktot-1)*2+2, index_w_i) ...
					res.M((ktot-1)*2+2, index_w_j)];
					
			Zk = diag([ params.sigma params.sigma alpha_i_var alpha_j_var]);
			
			Covk = Rk * Zk * Rk';
			
			
			Lk = [ proj 0; 0 0 1];
			Yk = [ proj*T_hat; phi_hat];
		
			res.s{ktot}.Lk = Lk;
			res.s{ktot}.Yk = Yk;
			res.s{ktot}.Rk = Rk;
			res.s{ktot}.Covk = Covk;
			
			ktot = ktot+1;
		end
		
%		fprintf('corr = %d \n', size(corre,2));
	
	end
	
	num_corr = ktot-1; 
	res.M = res.M(1:num_corr*2, 1:num_noises);
	res.Y = res.Y(1:num_corr*2, 1);
	res.L = res.L(1:num_corr*2, 1:3);
	
	% trova soluzione singola
	I = zeros(3,3);
	A = zeros(3,1);
	for i=1:num_corr
		I = I + res.s{i}.Lk' * inv(res.s{i}.Covk) * res.s{i}.Lk;
		A = A + res.s{i}.Lk' * inv(res.s{i}.Covk) * res.s{i}.Yk;
	end
	
	res.I = I;
	res.Cov = inv(I);
	res.x_hat = res.Cov * A;
	
function res = rot(phi)
	% Rotation matrix
	res = [cos(phi) -sin(phi); sin(phi) cos(phi)];
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';

