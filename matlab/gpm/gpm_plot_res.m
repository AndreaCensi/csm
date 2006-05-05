function gpm_plot_res(res, mmin, mmax)
	

	length = 0.5;
	hold on
	
	% number of correspondences
	Ktot = size(res.corr, 2);

	drawn=0;
	for k=1:Ktot
	
		
		drawn = drawn + 1;
		
		T = res.corr{k}.T;
		alpha = res.corr{k}.alpha;
		
		p1 = T + vers(alpha-pi/2) * length;
		p2 = T + vers(alpha+pi/2) * length;
		
		reference = [0;0;0;]; % f.ts.laser_ref.estimate;
		plotVectors(reference, [p1 p2], 'b-');
		
	end
	
	fprintf('drawn %d/%d\n', drawn, Ktot);
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';
