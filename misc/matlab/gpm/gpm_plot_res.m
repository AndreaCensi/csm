function gpm_plot_res(res, mmin, mmax)
	

	side = 2;
	length = sqrt(2)*side;
	hold on
	
	% number of correspondences
	Ktot = size(res.corr, 2);

	drawn=0;
	for k=1:Ktot
	
		
		drawn = drawn + 1;
		
		T = res.corr{k}.T;
		alpha = res.corr{k}.alpha;
		
		p1 = T + vers(alpha-pi/2) * length * 0.5;
		p2 = T + vers(alpha+pi/2) * length * 0.5;
		
		reference = [0;0;0;]; % f.ts.laser_ref.estimate;
		plotVectors(reference, [p1 p2], 'b-');
		
		axis([-side side -side side]/2);
	end
	
	fprintf('drawn %d/%d\n', drawn, Ktot);
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';
