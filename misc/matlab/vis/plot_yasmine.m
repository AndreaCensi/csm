function plot_yasmine(f, mmin, mmax)
	

	length = 0.5;
	hold on
	
	% number of correspondences
	Ktot = size(f.result.s, 2);

	drawn=0;
	for k=1:Ktot
	%	if f.result.s{k}.m  > mmax 
	%		continue
	%	end
		
		drawn = drawn + 1;
		
		T = f.result.s{k}.T;
		alpha = f.result.s{k}.alpha;
		
		p1 = T + vers(alpha-pi/2) * length;
		p2 = T + vers(alpha+pi/2) * length;
		
		reference = f.ts.laser_ref.estimate;
		plotVectors(reference, [p1 p2], 'b-');
		
	end
	
	fprintf('drawn %d/%d\n', drawn, Ktot);
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';
