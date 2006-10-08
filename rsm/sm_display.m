function [laser_ref, laser_sens] = sm_display(file)

cells = readFileInCells(file);
%global cells
[rows, columns] = size(cells)

% todo: remove comments

r = 1

r = skip_to(cells, r,'laser')
[r, laser_ref] = read_laser_data(cells, r);

r = skip_to(cells, r,'laser')
[r, laser_sens] = read_laser_data(cells, r);

f =  figure; hold on; axis('equal')

while true
	r = skip_to(cells, r,'iteration');
	if r > rows 
		break
	end
	
	r = skip_to(cells, r,'x_old');
	x_old = cells_to_vector(cells, r, 2);

	[corr_exist, r] = exist_this_iteration(cells, r,'correspondences');
    if corr_exist
        for i=1:laser_sens.nrays
            corr(i) = str2num(cells{r,1+i});
            if corr(i) == -1
                corr(i) = nan;
            else 
                % matlab counts from 1.. (yuk!)
                corr(i) = corr(i) + 1;
            end
        end
    end

	laser_ref.estimate = [0;0;0];
	laser_sens.estimate = x_old;

	params.plotNormals = true;
	params.color = 'r.';
	ld_plot(laser_ref, params);
	
	params.plotNormals = true;
	params.color = 'g.';
	ld_plot(laser_sens, params);
	
   if corr_exist
	for i=1:size(corr,2)
		if isnan(corr(i))
			continue
		end
		plot_line(transform(laser_sens.points(:,i),x_old), ...
			laser_ref.points(:,corr(i)),'k-');
    end
    end

	pause
	old_axis = axis
	clf
	axis(old_axis)
		
	%r = skip_to(cells, r,'x_new');
	%x_new = cells_to_vector(cells, r, 2)

end

function plot_line(a,b,color)
	plot([a(1) b(1)],[a(2) b(2)],color);

function x = cells_to_vector(cells, r, cell)
	for i=1:3
		x(i,1) = str2double(cells{r, cell+i-1});
	end

% Finds next row in cells whose first cell is equal to "cellname"
function r = skip_to(cells, r, cellname)
	while  (r<=size(cells,1)) && (0==strcmp(cells{r,1}, cellname))
		r = r +1;
    end

function [exist, r] = exist_this_iteration(cells, r, cellname);
	start =r;
    next_iteration = skip_to(cells, r, 'iteration');
    r = skip_to(cells, r, cellname)
    if r < next_iteration
        exist = true;
    else
			r = start;
        exist = false;
    end
        

function [next_r, laser_data] = read_laser_data(cells, r)

	name = cells{r, 2}
	ld = struct;
	ld.estimate = [0;0;0]
	ld.odometry = [0;0;0]
	while (r<=size(cells,1)) && strcmp(cells{r, 1},'laser') && strcmp(cells{r, 2},name)
		fprintf('at %s %s %s\n',cells{r,1},cells{r,2},cells{r,3})
		if strcmp(cells{r, 3}, 'min_theta')
			min_theta = str2double(cells{r,4});
		end
		if strcmp(cells{r, 3}, 'max_theta')
			max_theta = str2double(cells{r,4});
		end
		if strcmp(cells{r, 3}, 'nrays')
			ld.nrays = str2double(cells{r,4});
		end
		if strcmp(cells{r, 3}, 'valid')
			for i=1:ld.nrays
				ld.valid(i) = str2num(cells{r,3+i});
			end
		end
		if strcmp(cells{r, 3}, 'readings')
			for i=1:ld.nrays
				ld.readings(i) = str2double(cells{r,3+i});
				ld.theta(i) = min_theta + (max_theta-min_theta)*i/ld.nrays;
			end
		end
		if strcmp(cells{r, 3}, 'alpha_valid')
			for i=1:ld.nrays
				ld.alpha_valid(i) = str2num(cells{r,3+i});
			end
		end
		if strcmp(cells{r, 3}, 'alpha')
			for i=1:ld.nrays
				ld.alpha(i) = str2double(cells{r,3+i});
			end
		end
		if strcmp(cells{r, 3}, 'cov_alpha')
			for i=1:ld.nrays
				ld.cov_alpha(i) = str2double(cells{r,3+i});
			end
		end
		if strcmp(cells{r, 3}, 'cluster')
			for i=1:ld.nrays
				ld.cluster(i) = str2num(cells{r,3+i});
			end
		end
		r = r+1;
	end 
	
	ld.points = [cos(ld.theta) .* ld.readings; sin(ld.theta) .* 	ld.readings];
	
	next_r = r;
	laser_data =ld;
	
	
	
	