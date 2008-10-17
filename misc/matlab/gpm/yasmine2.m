% params.sigma
% params.laser_sens     % sensor (indexed by i)
% params.laser_ref      % map    (indexed by j)  laser_sens * (x,y,theta) = laser_ref 
% params.maxAngularCorreectionDeg
% params.maxLinearCorreection

function res = yasmine2(params)

	% readings for which a valid alpha was computed
	valid1 = find(params.laser_sens.alpha_valid);
	n_i = sum(params.laser_sens.alpha_valid)
	
	ktot = 1;
	
	for i=valid1
		
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
		
%			res.L((ktot-1)*2+1:(ktot-1)*2+2,1:3) = Lk;
%			res.Y((ktot-1)*2+1:(ktot-1)*2+2,1)   = Yk;
			
			index_e_i = (i-1)*4 + 1;
			index_w_i = (i-1)*4 + 2;
			index_e_j = (j-1)*4 + 3;
			index_w_j = (j-1)*4 + 4;
			
			v_i = vers(theta_i);
			v_j = vers(theta_j);

			% incertezza proj
			C = sqrt(size(corre,2));
			C = 1;
			r_e_i = C * (- proj * rot(phi_hat) * v_i);
			r_e_j = C * (  proj                * v_j);
			r_w_i = C * ( ...
				vers(alpha_j+pi/2)'*T_hat + ...
				proj *  rot(phi_hat+pi/2) * p_i );
			r_w_j = C * (- proj * rot(phi_hat+pi/2) * p_i);
			% incertezza angolo
			s_w_i = C * -1;
			s_w_j = C * 1; 
			
			Rk = [r_e_i r_e_j r_w_i r_w_j;...
					0 0 s_w_i s_w_j];
					
			Zk = diag([ params.sigma params.sigma alpha_i_var alpha_j_var]);
			
			Covk = Rk * Zk * Rk';
			
			Lk = [ proj 0; 0 0 1];
			Yk = [ proj*T_hat; phi_hat];
		
			maha = compute_ma(Lk,Yk,Covk, params.odometry, params.odometry_cov);
			if(maha>params.chi_limit)
				continue
			end

			res.s{ktot}.Lk = Lk;
			res.s{ktot}.Yk = Yk;
			res.s{ktot}.Rk = Rk;
			res.s{ktot}.Covk = Covk;
			res.s{ktot}.alpha = alpha_j;
			res.s{ktot}.T = T_hat;
			res.s{ktot}.phi = phi_hat;
			
			ktot = ktot+1;
		end
		
%		fprintf('corr = %d \n', size(corre,2));
	
	end
	
	
	trim = params.trim;
	chi_perc = params.chi_perc;
	chi_limit = params.chi_limit;
	
	valids = ones(1,ktot-1); valids_i = find(valids>0); nvalids = sum(valids);
	[x, Cov] = find_solution(res.s, valids_i, params.odometry, params.odometry_cov);
	
	limit = 2.3 * n_i
	while true
		if nvalids < limit
			res.x_hat = x;
			res.Cov = Cov;
			fprintf('Terminated reaching limit \n');
			break;
		end
		
		ms = compute_m(res.s, valids_i, x, Cov);
		
		fprintf(' valid = %d min %f mean %f max %f \n', nvalids, min(ms(valids_i)), ...
			mean(ms(valids_i)),max(ms(valids_i)));
		
		s = sum( ms(valids_i) <chi_limit );
		low_perc = s / nvalids;
		
%		fprintf(' low perc: %f \n', low_perc);
		if low_perc > chi_perc
			res.x_hat = x;
			res.Cov = Cov;
			break;
		end
		
		
		
		Y = sort(ms(valids_i),2,'descend');
		threshold = Y( ceil( trim *nvalids) );
		
%		fprintf(' threshold: %f \n', threshold);
		for i=valids_i
			if(ms(i)>threshold) 
				valids(i) = 0;
			end
		end
		
		valids_i = find(valids>0); nvalids = sum(valids);
	
		[x, Cov] = find_solution(res.s, valids_i, params.odometry, params.odometry_cov);
		fprintf(' x =  %f %f %f \n ', x(1),x(2),rad2deg(x(3)));

	end
	
	
	xy=sqrt(eig(Cov(1:2,1:2)));
	et=rad2deg(sqrt(Cov(3,3)));
	fprintf(' x =  %f %f %f \n ', x(1),x(2),rad2deg(x(3)));
	fprintf(' eig xy: %f %f \n ', xy(1), xy(2));
	fprintf(' eig theta: %f \n ', et(1));

function ms = compute_m(s, valids_i, x_hat, x_cov)
	for k=valids_i
		L = s{k}.Lk;
		Y = s{k}.Yk;
		Cov = s{k}.Covk; 
		ms(k) = compute_ma(L,Y,Cov, x_hat, x_cov);
	end

function ma = compute_ma(L,Y,C, x_hat, x_cov)
	Ctot = C + L * x_cov * L';
	e = L * x_hat - Y;
	ma = e' * inv(Ctot) * e;

function [x, Cov] = find_solution(s, valids_i, odometry, odometry_cov)
	% trova soluzione singola
	I = zeros(3,3);
	A = zeros(3,1);
	used = 0;
	for i=valids_i
		used = used +1;
		I = I + s{i}.Lk' * inv(s{i}.Covk) *s{i}.Lk;
		A = A + s{i}.Lk' * inv(s{i}.Covk) *s{i}.Yk;
	end
	
	I = I + inv(odometry_cov);
	A = A + inv(odometry_cov) * odometry;
	
	Cov = inv(I);
	x = Cov * A;
	%fprintf('used %d / %d \n', used, size(valids,2));

	
function res = rot(phi)
	% Rotation matrix
	res = [cos(phi) -sin(phi); sin(phi) cos(phi)];
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';

