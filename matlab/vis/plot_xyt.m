
function fs = plot_xyt(m) 
	
		p.datasets = m.datasets;
		p.covariances = m.covariances;
		p.scale = diag([1000 1000 180/pi]);
		p.format = m.format;
		p.legend = m.legend;
		
		p.select = [1 0 0; 0 1 0];;
		p.equal = true;
		p.xlabel = 'x (mm)';
		p.ylabel = 'y (mm)';
		p.title = sprintf('%s x, y',m.title);
		p.filename = sprintf('%s_xy.%s',m.prefix,m.extension);
		f1= draw_data_single(p);
		
		p.select = [1 0 0; 0 0 1];
		p.equal = false;
		p.xlabel = 'x (mm)';
		p.ylabel = '\phi (deg)';
		p.title = sprintf('%s x, \\phi',m.title);
		p.filename = sprintf('%s_xt.%s',m.prefix,m.extension);
		f2=draw_data_single(p);
	
		p.select = [0 0 1; 0 1 0];
		p.equal = false;
		p.xlabel = '\phi (deg)';
		p.ylabel = 'y (mm)';
		p.title = sprintf('%s \\phi, y',m.title);
		p.filename = sprintf('%s_ty.%s',m.prefix,m.extension);
		f3=draw_data_single(p);
	
		
		%set(f, 'PaperType','A5');
		%set(f, 'PaperUnits', 'centimeters');
		%set(f, 'PaperSize', [7 7]);
		%get(f)
		
		fs=[f1 f2 f3];

function f = draw_data_single(p)
	f = figure;
	hold on;
	select = p.select;
	scale = p.scale;
		for i=1:size(p.datasets,2)
			points = p.datasets{i}.points;
			color = p.datasets{i}.color;
			my_points(plot_points( select*scale*points, color ));
		end
		
		for i=1:size(p.covariances,2)
			mean = p.covariances{i}.mean;
			cov  = p.covariances{i}.cov;
			color = p.covariances{i}.color;
			bold_lines(plotGauss2Db(select*scale*mean, ...
			select*scale*cov*scale'*select', color)); 
		end
		
		my_fonts(legend(p.legend));
		if p.equal
			axis('equal');
		end
		
		my_fonts(xlabel(p.xlabel));
		my_fonts(ylabel(p.ylabel));
		my_fonts(title(p.title));
		my_fonts_small(gca);
	
		for i=1:size(p.covariances,2)
			mean = p.covariances{i}.mean;
			color = p.covariances{i}.color;
			plot_cross(select*scale*mean,color);
		end
		
		print(p.format,p.filename);
		
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

