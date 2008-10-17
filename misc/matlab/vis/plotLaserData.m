function plotLaserData(ld, params)
%  plotLaserData(ld, params)
%		Draws on current figure
%		
%		params.plotNormals = false;	
%		params.color = 'r.';
%		params.rototranslated (= true); if true, the scan is drawn
%			rototranslated at ld.estimate, else is drawn at 0;
%		params.rototranstated_odometry = false;
	
	if(nargin==1)
		params.plotNormals = false;	
		params.color = 'r.';
		params.rototranslated = true;
		params.rototranslated_odometry = false;
	end
	
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
	
	plotVectors(reference, ld.points, params.color);
	
	if params.plotNormals 
		% disegno normali
		maxLength = 0.05;
		
		valids = find(ld.alpha_valid);
		
		valid_points = ld.points(:,valids);
		valid_alpha  = ld.alpha(valids);
		valid_errors = rad2deg(sqrt(ld.alpha_error(valids)));;
		emin = min(valid_errors);
		emax = max(valid_errors);
		
		for i=1:size(valids,2)
			weight = 1 + valid_errors(i) * maxLength;
			
			v = [cos(valid_alpha(i)); sin(valid_alpha(i))] * weight;
			from = valid_points(:,i);
			to = from + v;
			plotVectors( reference, [from to] , 'g-');
		end
	end
