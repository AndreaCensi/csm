
function res = icp(params)
% params.laser_ref                - first scan
% params.laser_sens               - second scan
% params.maxAngularCorrectionDeg  - search space bound for phi, in degrees
% params.maxLinearCorrection      - search space bound for |t|, in degrees

	if false
		A = [1;0]; B= [0;1];
		p = [0;0];
		fprintf('test1: %s\n', pv(projection_on_line_seg(A,B,p)));
	
		fprintf('test1: %s\n', pv(closest_point_on_segment(A,B,p)));
	
		pause
	end
	
	
	params_required(params, 'laser_sens');
	params_required(params, 'laser_ref');
	params = params_set_default(params, 'maxAngularCorrectionDeg', 105);
	params = params_set_default(params, 'maxCorrespondenceDist', 4);
	params = params_set_default(params, 'maxLinearCorrection',    2);
	params = params_set_default(params, 'maxIterations',           40);
	params = params_set_default(params, 'firstGuess',         [0;0;deg2rad(10)]);
	params = params_set_default(params, 'interactive',  false);
	params = params_set_default(params, 'epsilon_xy',  0.00001);
	params = params_set_default(params, 'epsilon_theta',  0.0005);
	

	current_estimate = params.firstGuess;
	params.laser_sens.estimate = current_estimate;
	
	if params.interactive
		f = figure; hold on
	end
	
	for n=1:params.maxIterations
		estimates{n} = current_estimate;
	
		if params.interactive
			clf
			pl.color = 'b.';
			ld_plot(params.laser_ref,pl);
			pl.color = 'r.';
			params.laser_sens.estimate = current_estimate;
			ld_plot(params.laser_sens,pl);
		end

		[P, valids] = icp_get_correspondences(params, current_estimate);
	
		fprintf('Valid corr.: %d\n', sum(valids));
		next_estimate = next_estimate(params, current_estimate, P, valids);

		delta = next_estimate-current_estimate;
		current_estimate = next_estimate;
		
		fprintf('Delta: %s\n', pv(delta));
		fprintf('Estimate: %s\n', pv(current_estimate));
		
		if (norm(delta(1:2)) < params.epsilon_xy) & ...
			(norm(delta(3))   < params.epsilon_theta) 
			break;
		end
			
		%	current_estimate = next_estimate(params, current_estimate1, P, valids) % dovrebbe essere 0
		
		if params.interactive
			%pause
		end
		cova = icp_covariance(params, current_estimate, P, valids);
		
		pause(0.01)
	end % iterations
	
	fprintf('Converged at iteration %d.\n', n);

	res = params;
	res.X = current_estimate;
	res.Cov = cova.Cov;
	res.Inf = inv(res.Cov);
	
	res.iteration = n;
	estimates{n+1} = current_estimate;
	res.estimates = estimates;
	
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

function next = next_estimate(params, current_estimate, P, valids)
	if sum(valids) < 2
		error('icp:run', sprintf('Only %d correspondences found.',sum(valids)));
	end

	next = current_estimate;

	k=1; e = 0;
	for a=find(valids)
		points1(:,k) = transform(params.laser_sens.points(:,a), current_estimate);
		points2(:,k) = P(:,a);
		
		if params.interactive
			plot_line(points1(:,k),points2(:,k),'g-');
		end
		
		e = e + norm(points1(:,k)-points2(:,k));
		k=k+1;
	end
	
	[pose, L, Y] = exact_minimization(points1, points2);
	
	fprintf('exact_min: %s ', pv(pose));
	
	next_phi = current_estimate(3) + pose(3);
	next_t  = transform(current_estimate(1:2,1), pose);
	next = [next_t; next_phi];
	
	fprintf('Pose: %s  error: %f\n', pv(next), e);
	
function plot_line(a,b,color)
	plot([a(1) b(1)],[a(2) b(2)], color);

function [P,valid] = icp_get_correspondences(params,current_estimate)
	for i=1:params.laser_sens.nrays
		p_i = params.laser_sens.points(:,i);
		p_i_w = transform(p_i, current_estimate);
		
		% find best correspondence
		best_j = 0; best_dist = 0;
		delta = 20;
		from = max(i-delta,1);
		to = min(i+delta,params.laser_ref.nrays);
		
		from = 1;
		to = params.laser_ref.nrays;
		
		for j=from:to
			% Find compatible interval in the other scan. 
			p_j = params.laser_ref.points(:,j);
			
			dist = norm( p_i_w - p_j);
			if dist < params.maxCorrespondenceDist
				if (best_j==0) || (dist < best_dist)
					best_j = j; best_dist = dist;
				end
			end
		end

		if best_j == 0
			P(:,i) = [nan;nan];
			valid(i) = 0;
		else
			% find other point to interpolate
			if best_j==1
				other = best_j+1;
			elseif best_j == params.laser_ref.nrays
				other = best_j-1;
			else
				p_prev = params.laser_ref.points(:,best_j-1);
				p_next = params.laser_ref.points(:,best_j+1);
				dist_prev = norm( p_prev-p_i_w);
				dist_next = norm( p_next-p_i_w);
				if dist_prev < dist_next
					other = p_prev;
				else
					other = p_next;
				end
			end
			
			% find point which is closer to segment
			interpolate = closest_point_on_segment(params.laser_ref.points(:,best_j),other, p_i_w);
	
			dist = norm(interpolate-p_i_w);
			if dist < params.maxCorrespondenceDist
				P(:,i) = interpolate;
				valid(i) = 1;
			else
				P(:,i) = [nan;nan];
				valid(i) = 0;
			end
		end
			
	end % i in first scan 
	
