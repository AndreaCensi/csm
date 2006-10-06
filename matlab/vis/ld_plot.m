
function res = ld_plot(ld, params)
%  plotLaserData(ld, params)
%		Draws on current figure
%		
%		params.plotNormals = false;	
%		params.color = 'r.';
%		params.rototranslated (= true); if true, the scan is drawn
%			rototranslated at ld.estimate, else is drawn at 0;
%		params.rototranstated_odometry = false;
	
	if(nargin==1)
		params.auto = false;
	end
	
	
	params = params_set_default(params, 'plotNormals', false);
	params = params_set_default(params, 'color',    'r.');
	
	params = params_set_default(params, 'rototranslated',  true);	
	params = params_set_default(params, 'rototranslated_odometry',  false);	
	
	params = params_set_default(params, 'plot_true_alpha', false);
	params = params_set_default(params, 'plot_true_alpha_length', 0.4);
	params = params_set_default(params, 'plot_true_alpha_color', 'g-');
	
	params = params_set_default(params, 'plot_rays', false);
	params = params_set_default(params, 'plot_rays_color', 'b-');
	params = params_set_default(params, 'plot_rays_interval', 10);

	if(params.rototranslated_odometry)
		reference = ld.odometry;	
	else
		if(params.rototranslated)
			reference = ld.estimate;	
		else
			reference = [0 0 0]';
		end
	end
		
	hold on
	
	res.points = plotVectors(reference, ld.points, params.color);
	
	if params.plot_rays
		for i=1:params.plot_rays_interval:ld.nrays
			plotVectors(reference, [0 0; ld.points(:,i)']', params.plot_rays_color);
		end
	end
	
	if params.plotNormals 
		% disegno normali
		maxLength = 0.05;
		
		valids = find(ld.alpha_valid);
        if not(isfield(ld,'alpha_error'))
            ld.alpha_error = ones(1, ld.nrays);
        end
		
		valid_points = ld.points(:,valids);
		valid_alpha  = ld.alpha(valids);
		valid_errors = rad2deg(sqrt(ld.alpha_error(valids)));;
		emin = min(valid_errors);
		emax = max(valid_errors);
		
		for i=1:size(valids,2)
			%weight = 1 + valid_errors(i) * maxLength;
			weight = maxLength;
            
			v = [cos(valid_alpha(i)); sin(valid_alpha(i))] * weight;
			from = valid_points(:,i);
			to = from + v;
			plotVectors( reference, [from to] , 'g-');
		end
	end
	
	if params.plot_true_alpha
		% disegno normali
		length = params.plot_true_alpha_length;
		color = params.plot_true_alpha_color;
		
		for i=1:ld.nrays
			v = [cos(ld.true_alpha(i)); sin(ld.true_alpha(i))] * length;
			from = ld.points(:,i);
			to = from + v;
			plotVectors( reference, [from to] , color);
		end
	end
