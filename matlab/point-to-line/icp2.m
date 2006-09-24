% Dependences of this script:
% Requires: params_required, params_set_default, icp_get_correspondences
% Requires: ld_plot, icp_covariance, exact_minimization, transform
% Requires: icp_possible_interval

function res = icp2(params)
% Note: it is assumed that params.laser_ref has a radial uniform scan 
% params.laser_ref                - first scan
% params.laser_sens               - second scan
% params.maxAngularCorrectionDeg  - search space bound for phi, in degrees
% params.maxLinearCorrection      - search space bound for |t|, in degrees
	
	
	params_required(params, 'laser_sens');
	params_required(params, 'laser_ref');
	params = params_set_default(params, 'maxAngularCorrectionDeg', 105);
	params = params_set_default(params, 'maxCorrespondenceDist', 4);
	params = params_set_default(params, 'maxLinearCorrection',    2);
	params = params_set_default(params, 'maxIterations',           40);
	params = params_set_default(params, 'firstGuess',         [0;0;0]);
	params = params_set_default(params, 'interactive',  false);
	params = params_set_default(params, 'epsilon_xy',  0.0001);
	params = params_set_default(params, 'epsilon_theta',  deg2rad(0.001));
	params = params_set_default(params, 'sigma',  0.01);
	params = params_set_default(params, 'do_covariance', false);

	%% If true, consider as null correspondences with first or last point
	%% in the reference scan.
	params = params_set_default(params, 'dont_consider_extrema', true);
	
	%params.laser_ref = computeSurfaceNormals(params.laser_ref);

	current_estimate = params.firstGuess;
	params.laser_sens.estimate = current_estimate;
	
	if params.interactive
		f = figure; hold on
	end

	
	estimated_cov = eye(3);	
	
	for n=1:params.maxIterations
		estimates{n} = current_estimate;
		
		%% If in interactive mode, show partial solution.
		if params.interactive
			clf
			pl.color = 'b.';
			ld_plot(params.laser_ref,pl);
			pl.color = 'r.';
			params.laser_sens.estimate = ...
				rtcat(params.laser_ref.estimate, current_estimate);
			ld_plot(params.laser_sens,pl);
			axis('equal');
		
		
			%for i=find(valids)
	
	%			plotVectors(params.laser_sens.estimate, [params.laser_sens.points(:,i) P(:,i)] , 'k');
	
			%end
		end
	

		[P, valids, jindexes] = icp_get_correspondences(params, current_estimate);
		
		fprintf('Valid corr.: %d\n', sum(valids));
		next_estimate = next_estimate(params, current_estimate, P, valids, jindexes);

		delta = next_estimate-current_estimate;
	
		
		current_estimate = next_estimate;
		
		fprintf('Delta: %s\n', pv(delta));
		fprintf('Estimate: %s\n', pv(current_estimate));
		
		if (norm(delta(1:2)) < params.epsilon_xy) & ...
			(norm(delta(3))   < params.epsilon_theta) 
			
			break;
		end
		
		if params.interactive
			pause
		end
		
		pause(0.01)
	end % iterations
fprintf('Converged at iteration %d.\n', n);

	res = params;
	
	if(params.do_covariance)		
		estimated_cov = icp_covariance(params, current_estimate, P, valids, jindexes);	
		res.sm_cov_bengtsson = estimated_cov.sm_cov_bengtsson;
		res.loc_cov_censi = estimated_cov.loc_cov_censi;
		res.sm_cov_censi = estimated_cov.sm_cov_censi;
	end
	
	
	res.X = current_estimate;
	res.iteration = n;
	estimates{n+1} = current_estimate;
	res.estimates = estimates;
	
	
function next = next_estimate(params, current_estimate, P, valids, jindexes)
% Requires: transform
	if sum(valids) < 2
		error('icp:run', sprintf('Only %d correspondences found.',sum(valids)));
	end

	next = current_estimate;

	corr={};
	k=0; e = 0;
	for i=find(valids)
		k=k+1;
		p_k = transform(params.laser_sens.points(:,i), current_estimate);
		q_k = P(:,i);
		
		j1 = jindexes(i,1);
		j2 = jindexes(i,2);

		p_j1 = params.laser_ref.points(:,j1);
		p_j2 = params.laser_ref.points(:,j2);
		
		v_alpha = rot(pi/2) * (p_j1-p_j2);
		v_alpha = v_alpha / norm(v_alpha);
		
		corr{k}.p = p_k;
		corr{k}.q = q_k;
		corr{k}.C = v_alpha * v_alpha';
	%% For point-to-point:
	%	corr{k}.C = eye(2);
		
		 
			
		if params.interactive
			a = transform(params.laser_sens.points(:,i), ...
				rtcat(params.laser_ref.estimate, current_estimate));
			b = transform(q_k, ...
					params.laser_ref.estimate);
		
			plot_line(a,b,'g-');
		end
		
		e = e + norm(p_k-q_k);
	end

	
	res = general_minimization(corr);
	pose = res.x;

	fprintf('exact min. (p2l): %s \n', pv(pose));
	
	if true
		npos = 0; nneg = 0;
		k2 = 0; corr2={};
		k3 = 0; corr3={};
		
		for k=1:size(corr,2)
			theta = pose(3); t =pose(1:2);
			grad = (rot(theta)*corr{k}.p + t - corr{k}.q)' * ...
			corr{k}.C * [eye(2) (rot(theta+pi/2) * corr{k}.p)];
			
			if grad * pose < 0
				k2 = k2 +1;
				corr2{k2} = corr{k};
				nneg = nneg + 1;
			else
				k3 = k3 +1;
				corr3{k3} = corr{k};
				npos = npos + 1;
				if params.interactive
					a = transform(corr{k}.p, params.laser_ref.estimate);
					b = transform(corr{k}.q, params.laser_ref.estimate);
					plot_line(a,b,'k-');
				end
			end
		end

		fprintf('pos: %d neg: %d\n', npos, nneg);	
		threshold = 0.1;
		if npos < threshold * nneg
			fprintf('Doing the trick!\n')
			res = general_minimization(corr2);
			pose = res.x;
			fprintf('exact min. after trick (p2l): %s \n', pv(pose));
		end
		if nneg < threshold * npos
			fprintf('Doing the trick! (other) \n')
			res = general_minimization(corr3);
			pose = res.x;
			fprintf('exact min. after trick (p2l): %s \n', pv(pose));		
		end
		if npos > nneg
			fprintf('Finished trick!\n')
		end
	end
	
	next_phi = current_estimate(3) + pose(3);
	next_t  = transform(current_estimate(1:2,1), pose);
	next = [next_t; next_phi];
	
	fprintf('Pose: %s  error: %f\n', pv(next), e);
	
function plot_line(a,b,color)
	plot([a(1) b(1)],[a(2) b(2)], color);


