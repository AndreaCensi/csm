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


while true
	r = skip_to(cells, r,'iteration');
	if r > rows 
		break
	end
	
	r = skip_to(cells, r,'x_old');
	x_old = cells_to_vector(cells, r, 2);

	r = skip_to(cells, r,'correspondences');
	for i=1:laser_sens.nrays
		corr(i) = str2num(cells{r,1+i});
		if corr(i) == -1
			corr(i) = nan;
		else 
			% matlab counts from 1.. (yuk!)
			corr(i) = corr(i) + 1;
		end
	end

	laser_ref.estimate = [0;0;0];
	laser_sens.estimate = x_old;
	
	f = figure; hold on; axis('equal')
	params.color = 'r.'
	ld_plot(laser_ref, params);
	params.color = 'g.'
	ld_plot(laser_sens, params);
	

	pause
	
	r = skip_to(cells, r,'x_new');
	x_new = cells_to_vector(cells, r, 2)


	r = r + 1;
end

function x = cells_to_vector(cells, r, cell)
	for i=1:3
		x(i,1) = str2double(cells{r, cell+i-1});
	end

% Finds next row in cells whose first cell is equal to "cellname"
function r = skip_to(cells, r, cellname)
	while  (r<=size(cells,1)) && (0==strcmp(cells{r,1}, cellname))
		r = r +1
	end


function [next_r, laser_data] = read_laser_data(cells, r)

	name = cells{r, 2}
	ld = struct;
	ld.estimate = [0;0;0]
	ld.odometry = [0;0;0]
	while strcmp(cells{r, 1},'laser') && strcmp(cells{r, 2},name)
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
		if strcmp(cells{r, 3}, 'readings')
			for i=1:ld.nrays
				ld.readings(i) = str2double(cells{r,3+i});
				ld.theta(i) = min_theta + (max_theta-min_theta)*i/ld.nrays;
			end
		end
		r = r+1;
	end 
	
	ld.points = [cos(ld.theta) .* ld.readings; sin(ld.theta) .* 	ld.readings];
	
	next_r = r;
	laser_data =ld;
	
	
	
	