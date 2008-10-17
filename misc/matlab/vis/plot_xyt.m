
function fs = plot_xyt(m) 
	
		p.datasets = m.datasets;
		p.covariances = m.covariances;
		p.scale = diag([1000 1000 180/pi]);
		p.format = m.format;
		p.legend = m.legend;
		
	
		p.use_legend = true;
		
		p.select = [1 0 0; 0 1 0];;
		p.equal = true;
		p.xlabel = 'x (mm)';
		p.ylabel = 'y (mm)';
		p.title = sprintf('%s x, y',m.title);
		p.filename = sprintf('%s_xy.%s',m.prefix,m.extension);
		f1= draw_data_single(p);
		
		p.use_legend = false;
		
		p.select = [1 0 0; 0 0 1];
		p.equal = false;
		p.xlabel = 'x (mm)';
		p.ylabel = '\theta (deg)';
		p.title = sprintf('%s x, \\theta',m.title);
		p.filename = sprintf('%s_xt.%s',m.prefix,m.extension);
		f2=draw_data_single(p);
	
		p.select = [0 0 1; 0 1 0];
		p.equal = false;
		p.xlabel = '\theta (deg)';
		p.ylabel = 'y (mm)';
		p.title = sprintf('%s \\theta, y',m.title);
		p.filename = sprintf('%s_ty.%s',m.prefix,m.extension);
		
		f3=draw_data_single(p);
	
		fs=[f1 f2 f3];

function f = draw_data_single(p)
	f = figure;
	
	fsize = 7;
	set(f,'Units','centimeters'); 
	set(f,'Position',[0 0 fsize fsize]); 
	set(f,'PaperUnits','centimeters'); 
	set(f,'PaperPosition',[0 0 fsize fsize]); 
	set(f,'PaperSize',[fsize fsize])
		
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
		
		if p.use_legend
			l = legend(p.legend);
			set(l, 'Orientation', 'vertical');
			set(l, 'Location','NorthWest');
			%get(l)
			my_fonts(l);
		end
		
		if p.equal
			axis('equal');
		end
		
		my_fonts(xlabel(p.xlabel));
		my_fonts(ylabel(p.ylabel));
		my_fonts_large(title(p.title));
		my_fonts_small(gca);
	
		for i=1:size(p.covariances,2)
			mean = p.covariances{i}.mean;
			color = p.covariances{i}.color;
			bold_lines(plot_cross(select*scale*mean,color));
		end
	
		print(p.format,p.filename);
		
function res = plot_points(points, color)
	res = plot(points(1,:),points(2,:),color);
	
function res = plot_cross(mean, color)
		A = axis;
		res(1) = plot([A(1) A(2)],[mean(2) mean(2)],color);
		res(2) = plot([mean(1) mean(1)], [A(3) A(4)],color);
		%res
		%size(res)

function bold_lines(h)
	for i=1:size(h,2)
		set(h(i), 'LineWidth', 2);	
	end

function my_fonts(h)
	set(h, 'fontunits', 'points');
	set(h, 'fontsize', 10);
	set(h, 'fontname', 'Times');

function my_fonts_large(h)
	set(h, 'fontunits', 'points');
	set(h, 'fontsize', 10);
	set(h, 'fontname', 'Times');

function my_fonts_small(h)
	set(h, 'fontunits', 'points');
	set(h, 'fontsize', 9);
	set(h, 'fontname', 'Times');

function my_points(h)
	set(h,'MarkerSize',10);

