% Dependences of this script:
% Requires: params_required, params_set_default, icp_get_correspondences
% Requires: ld_plot, icp_covariance, exact_minimization, transform
% Requires: icp_possible_interval

function res = icp(params)
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
	params = params_set_default(params, 'dont_consider_extrema', false);
	

	current_estimate = params.firstGuess;
	params.laser_sens.estimate = current_estimate;
	
	if params.interactive
		f = figure; hold on
	end

	
	estimated_cov = eye(3);	
	
	for n=1:params.maxIterations
		estimates{n} = current_estimate;
	

		[P, valids, jindexes] = icp_get_correspondences(params, current_estimate);
		
		fprintf('Valid corr.: %d\n', sum(valids));
		next_estimate = next_estimate(params, current_estimate, P, valids);

		delta = next_estimate-current_estimate;
	
		
		%% If in interactive mode, show partial solution.
		if params.interactive
			clf
			pl.color = 'b.';
			ld_plot(params.laser_ref,pl);
			pl.color = 'r.';
			params.laser_sens.estimate = rtcat(params.laser_ref.estimate, current_estimate);
			ld_plot(params.laser_sens,pl);
			axis('equal');
		
		
			for i=find(valids)
	
				plotVectors(params.laser_sens.estimate, [params.laser_sens.points(:,i) P(:,i)] , 'k');
	
			end
		
		%	pause
		end
		
		current_estimate = next_estimate;
		
		fprintf('Delta: %s\n', pv(delta));
		fprintf('Estimate: %s\n', pv(current_estimate));
		
		if (norm(delta(1:2)) < params.epsilon_xy) & ...
			(norm(delta(3))   < params.epsilon_theta) 
			
			break;
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
	
	
function next = next_estimate(params, current_estimate, P, valids)
% Requires: transform
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


