function an = test_icp_covariance1_ana(res, test_title, prefix)
	% analyzes result of the test
	
	truth =[0;0;0];
	
	for i=1:size(res,2)
		p(:,i) = res{i}.X - truth;
	end
	
	
	mean(p,2)
	cov(p')
	
	
	an.bias = mean(p,2);
	an.sample_cov = cov(p');
	
	an.p = p;
	
	crb = compute_bounds(res{1}.laser_ref);
	
	sigma = 0.01;
	an.crb = crb.C0 * (sigma^2);
	an.estimated_cov = res{1}.Cov * (sigma^2);
	
	draw_estimate = false;
	print_figures = true;
		format = '-depsc2';
		format = '-depsc2';
		
	color_samples = 'r.';
	color_samples_cov = 'r-';
	color_crb = 'b--';
	color_est = 'b:';
	
	
	zero = [0;0;0];
	scale = diag([1000 1000 180/pi]);
	points = p;
	
	% draw xy
	
	select = [1 0 0; 0 1 0];
		f = figure; hold on;
		my_points(plot_points( select*scale*points, color_samples ));
		bold_lines(plotGauss2Db(select*scale*an.bias, ...
			select*scale*an.sample_cov*scale'*select', color_samples_cov)); 
		bold_lines(plotGauss2Db(select*scale*zero, ...
			select*scale*an.crb*scale'*select', color_crb)); 
		if draw_estimate
			bold_lines(plotGauss2Db(select*scale*zero, ...
				select*scale*an.estimated_cov*scale'*select',color_est)); 
		end
	
		my_fonts(legend('Samples', 'Sample \Sigma', 'Cramer-Rao',...
			'Location','BestOutside'));
		axis('equal');
		plot_cross(select*scale*an.bias,color_samples_cov);
		plot_cross(select*scale*zero,color_crb);
		my_fonts(xlabel('Error on x (mm)'));
		my_fonts(ylabel('Error on y (mm)'));
		my_fonts(title(sprintf('%s Errors on x, y',test_title)));
		my_fonts_small(gca);
		
		suffix = '_xy.eps';
		filename = sprintf('%s%s',prefix,suffix);
		print(format,filename);
		
	select = [1 0 0; 0 0 1];
	
		f = figure; hold on;
		my_points(plot_points( select*scale*points, color_samples ));
		bold_lines(plotGauss2Db(select*scale*an.bias, ...
			select*scale*an.sample_cov*scale'*select', color_samples_cov)); 
		bold_lines(plotGauss2Db(select*scale*zero, ...
			select*scale*an.crb*scale'*select', color_crb)); 
		if draw_estimate
			bold_lines(plotGauss2Db(select*zero, ...
				select*scale*an.estimated_cov*scale'*select',color_est)); 
		end
		
		my_fonts(legend('Samples', 'Sample \Sigma', 'Cramer-Rao',...
			'Location','BestOutside'));
		my_fonts(title(sprintf('%s Errors on x, \\phi',test_title)));
		my_fonts(xlabel('Error on x (mm)'));
		my_fonts(ylabel('Error on \phi (deg)'));
		my_fonts_small(gca);
		plot_cross(select*scale*an.bias,color_samples_cov);
		plot_cross(select*scale*zero,color_crb);
		
		suffix = '_xt.eps';
		filename = sprintf('%s%s',prefix,suffix);
		print(format,filename);
		
		
	select = [0 0 1; 0 1 0];
		f = figure; hold on;
		
		my_points(plot_points( select*scale*points, color_samples ));
		bold_lines(plotGauss2Db(select*scale*an.bias, ...
			select*scale*an.sample_cov*scale'*select', color_samples_cov)); 
		bold_lines(plotGauss2Db(select*scale*zero, ...
			select*scale*an.crb*scale'*select', color_crb)); 
		if draw_estimate
			bold_lines(plotGauss2Db(select*scale*zero, ...
				select*scale*an.estimated_cov*scale'*select',color_est)); 
		end
		
		my_fonts(legend('Samples', 'Sample \Sigma', 'Cramer-Rao',...
			'Location','BestOutside'));
		my_fonts(title(sprintf('%s Errors on \\phi, y',test_title)));
		my_fonts(xlabel('Error on \phi (deg)'));
		my_fonts(ylabel('Error on y (mm)'));
		my_fonts_small(gca);
		plot_cross(select*scale*an.bias,color_samples_cov);
		plot_cross(select*scale*zero,color_crb);
	
		suffix = '_ty.eps';
		filename = sprintf('%s%s',prefix,suffix);
		print(format,filename);
		
function res = plot_points(points, color)
	res = plot(points(1,:),points(2,:),color);
	
function res = plot_cross(mean, color)
		A = axis;
		res(1) = plot([A(1) A(2)],[mean(2) mean(2)],color);
		res(2) = plot([mean(1) mean(1)], [A(3) A(4)],color);
	
function bold_lines(h)
	for i=1:size(h)
		set(h(i), 'LineWidth', 3);	
	end

function my_fonts(h)
	set(h, 'fontunits', 'points');
	set(h, 'fontsize', 18);
	set(h, 'fontname', 'Times');

function my_fonts_small(h)
	set(h, 'fontunits', 'points');
	set(h, 'fontsize', 14);
	set(h, 'fontname', 'Times');

function my_points(h)
	set(h,'MarkerSize',14);

function res = mycov(P)

	res = zeros(size(P,1));
	for i=1:size(P,2)
		res = res + P(:,i) * P(:,i)';
	end
	res = res / (1-size(P,2));
